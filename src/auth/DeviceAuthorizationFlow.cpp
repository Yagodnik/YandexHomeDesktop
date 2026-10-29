#include "DeviceAuthorizationFlow.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QElapsedTimer>
#include <QUrlQuery>
#include <cmath>

struct DeviceAuthorizationFlow::Attempt : QObject {
  using QObject::QObject;
  QPointer<QObject> context;
  AuthResultHandler<QString> handler;
  QPointer<QNetworkReply> reply;
  QUrl codeUrl, tokenUrl;
  QString clientId, secret, deviceCode;
  QByteArray scope;
  int interval = 5;
  int expires = 0;
  QElapsedTimer elapsed;
};

namespace {
AuthResult<QString> Failure(AuthErrorKind kind, quint32 code) {
  return std::unexpected(AuthError{kind, code, {}});
}
QByteArray Form(const QUrlQuery& query) { QByteArray result;
  for (const auto& item : query.queryItems(QUrl::FullyDecoded)) {
    if (!result.isEmpty()) result += '&';
    result += QUrl::toPercentEncoding(item.first) + '=' + QUrl::toPercentEncoding(item.second);
  }
  return result; }
QNetworkRequest Request(const QUrl& url) {
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
  request.setTransferTimeout(30000);
  return request;
}
bool Https(const QUrl& url) {
  return url.isValid() && url.scheme() == "https" && !url.host().isEmpty() &&
      url.userInfo().isEmpty() && !url.hasFragment();
}
}

DeviceAuthorizationFlow::DeviceAuthorizationFlow(QObject* parent, QString path, QNetworkAccessManager* network)
    : QObject(parent), config_path_(std::move(path)), network_(network ? network : new QNetworkAccessManager(this)) {}
DeviceAuthorizationFlow::~DeviceAuthorizationFlow() { Cancel(); }
void DeviceAuthorizationFlow::Start(QObject* context, AuthResultHandler<QString> handler) {
  Cancel();
  auto* a = new Attempt(this); attempt_ = a; a->context = context; a->handler = std::move(handler);
  connect(context, &QObject::destroyed, a, [this, a] { if (attempt_ == a) Cancel(); });
  QTimer::singleShot(0, a, [this, a] {
    QFile file(config_path_);
    if (!file.open(QIODevice::ReadOnly)) { Finish(a, Failure(AuthErrorKind::Initialization, 0x80010002u)); return; }
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    const auto obj = doc.object();
    const auto id = obj.value("client_id"); const auto secret = obj.value("client_secret");
    const auto token = obj.value("access_token_url"); const auto scopes = obj.value("scopes");
    QUrl url(token.toString(), QUrl::StrictMode);
    if (error.error != QJsonParseError::NoError || !doc.isObject() || !id.isString() || id.toString().trimmed().isEmpty() ||
        !secret.isString() || !token.isString() || !Https(url) || !scopes.isArray()) {
      Finish(a, Failure(AuthErrorKind::Initialization, 0x80010001u)); return;
    }
    QStringList scopeList;
    for (const auto& value : scopes.toArray()) {
      if (!value.isString() || value.toString().contains(' ')) { Finish(a, Failure(AuthErrorKind::Initialization, 0x80010001u)); return; }
      if (!value.toString().isEmpty()) scopeList << value.toString();
    }
    a->clientId = id.toString(); a->secret = secret.toString(); a->scope = scopeList.join(',').toUtf8();
    url.setQuery(QString()); url.setPath("/token"); a->tokenUrl = url;
    url.setPath("/device/code"); a->codeUrl = url;
    RequestCode(a);
  });
}
void DeviceAuthorizationFlow::RequestCode(Attempt* a) {
  QUrlQuery form; form.addQueryItem("client_id", a->clientId);
  if (!a->scope.isEmpty()) form.addQueryItem("scope", QString::fromUtf8(a->scope));
  auto* reply = network_->post(Request(a->codeUrl), Form(form)); a->reply = reply;
  connect(reply, &QNetworkReply::finished, a, [this, a, reply] {
    const auto body = reply->readAll(); const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater(); a->reply = nullptr;
    QJsonParseError error; const auto doc = QJsonDocument::fromJson(body, &error); const auto obj = doc.object();
    const auto code = obj.value("device_code"); const auto user = obj.value("user_code");
    const auto verification = obj.value("verification_url"); const QUrl url(verification.toString(), QUrl::StrictMode);
    const auto interval = obj.value("interval"); const auto expiry = obj.value("expires_in");
    const double seconds = interval.toDouble(); const double lifetime = expiry.toDouble();
    if (status != 200 || error.error != QJsonParseError::NoError || !doc.isObject() ||
        !code.isString() || code.toString().isEmpty() || !user.isString() || user.toString().isEmpty() ||
        !verification.isString() || !Https(url) || url.host().compare(a->codeUrl.host(), Qt::CaseInsensitive) != 0 ||
        !interval.isDouble() || seconds < 1 || seconds > 3600 || std::floor(seconds) != seconds ||
        !expiry.isDouble() || lifetime < 1 || lifetime > 86400 || std::floor(lifetime) != lifetime) {
      Finish(a, Failure(AuthErrorKind::Authorization, 0x80010005u)); return;
    }
    a->deviceCode = code.toString(); a->interval = int(seconds); a->expires = int(lifetime); a->elapsed.start();
    if (prompt_) prompt_(user.toString(), url.toString());
    if (attempt_ != a) return;
    QTimer::singleShot(a->expires * 1000, a, [this, a] { Finish(a, Failure(AuthErrorKind::Authorization, 0x80010006u)); });
    QTimer::singleShot(a->interval * 1000, a, [this, a] { Poll(a); });
  });
}
void DeviceAuthorizationFlow::Poll(Attempt* a) {
  if (attempt_ != a) return;
  if (a->elapsed.elapsed() >= qint64(a->expires) * 1000) { Finish(a, Failure(AuthErrorKind::Authorization, 0x80010006u)); return; }
  QUrlQuery form; form.addQueryItem("grant_type", "device_code"); form.addQueryItem("code", a->deviceCode);
  form.addQueryItem("client_id", a->clientId); form.addQueryItem("client_secret", a->secret);
  auto* reply = network_->post(Request(a->tokenUrl), Form(form)); a->reply = reply;
  connect(reply, &QNetworkReply::finished, a, [this, a, reply] {
    const auto body = reply->readAll(); const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater(); a->reply = nullptr;
    QJsonParseError parse; const auto doc = QJsonDocument::fromJson(body, &parse); const auto obj = doc.object();
    if (a->elapsed.elapsed() >= qint64(a->expires) * 1000) { Finish(a, Failure(AuthErrorKind::Authorization, 0x80010006u)); return; }
    if (status == 200 && parse.error == QJsonParseError::NoError && obj.value("access_token").isString() && !obj.value("access_token").toString().isEmpty()) {
      Finish(a, obj.value("access_token").toString()); return;
    }
    const auto error = obj.value("error").toString();
    if (status >= 400 && status < 500 && (error == "authorization_pending" || error == "slow_down")) {
      if (error == "slow_down") a->interval = qMin(a->interval + 5, 3600);
      QTimer::singleShot(a->interval * 1000, a, [this, a] { Poll(a); }); return;
    }
    Finish(a, Failure(error == "access_denied" ? AuthErrorKind::Canceled : AuthErrorKind::Authorization, 0x80010007u));
  });
}
void DeviceAuthorizationFlow::Finish(Attempt* a, AuthResult<QString> result) {
  if (attempt_ != a) return;
  auto handler = std::move(a->handler); auto context = a->context; Cancel();
  if (context) handler(std::move(result));
}
void DeviceAuthorizationFlow::Cancel() {
  if (!attempt_) return;
  auto* a = attempt_; attempt_ = nullptr;
  if (a->reply) { a->reply->disconnect(a); a->reply->abort(); a->reply->deleteLater(); }
  a->deleteLater();
}
