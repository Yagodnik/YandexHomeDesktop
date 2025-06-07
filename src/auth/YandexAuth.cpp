#include "YandexAuth.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QFile>
#include <QDesktopServices>

YandexAuth::YandexAuth(QObject *parent) : QObject{parent}, reply_handler_(kDefaultPort) {
  const auto auth_secrets_object = GetAuthSecrets();

  if (!auth_secrets_object.has_value() || !PrepareCallbackPage()) {
    emit initializationFailed();
    return;
  }

  auto auth_secrets = Serialization::From<AuthSecrets>(
    auth_secrets_object.value());

  oauth2_.setReplyHandler(&reply_handler_);
  oauth2_.setAuthorizationUrl({auth_secrets.auth_url});
  oauth2_.setTokenUrl({auth_secrets.access_token_url});
  oauth2_.setClientIdentifier(auth_secrets.client_id);
  oauth2_.setClientIdentifierSharedKey(auth_secrets.client_secret);
  oauth2_.setRequestedScopeTokens(GetScopes(auth_secrets.scopes));

  connect(&oauth2_,
    &QOAuth2AuthorizationCodeFlow::statusChanged,
    this,
    &YandexAuth::HandleAuthorizationStatus);

  connect(&oauth2_,
    &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
    this,
    &YandexAuth::AuthorizeWithBrowser);
}

bool YandexAuth::IsAuthorized() const {
  return token_.has_value();
}

void YandexAuth::AttemptAuthorization() {
  oauth2_.grant();
}

std::optional<QJsonObject> YandexAuth::GetAuthSecrets() const {
  QFile auth_secrets_file(kAuthSecretsPath);
  if (!auth_secrets_file.open(QIODevice::ReadOnly)) {
    return std::nullopt;
  }

  const QByteArray data = auth_secrets_file.readAll();
  auth_secrets_file.close();

  QJsonDocument document = QJsonDocument::fromJson(data);
  return document.object();
}

QSet<QByteArray> YandexAuth::GetScopes(const QStringList &list) {
  QSet<QByteArray> result;

  for (const QString &scope : list) {
    result.insert(scope.toUtf8());
  }

  return result;
}

bool YandexAuth::PrepareCallbackPage() {
  QFile callback_index(kCallbackPath);
  if (!callback_index.open(QIODevice::ReadOnly)) {
    return false;
  }

  reply_handler_.setCallbackText(callback_index.readAll());
  callback_index.close();

  return true;
}

void YandexAuth::HandleAuthorizationStatus(const QAbstractOAuth::Status status) {
  switch (status) {
    case QAbstractOAuth::Status::Granted:
      token_ = oauth2_.token();
    emit authorized();
    break;
    case QAbstractOAuth::Status::NotAuthenticated:
      emit authorizationFailed();
    break;
    case QAbstractOAuth::Status::RefreshingToken:
      qDebug() << "Auth: Refreshing token";
    break;
    default:
      qDebug() << "Auth: Unknown status";
    break;
  }
}

void YandexAuth::AuthorizeWithBrowser(QUrl url) {
  QUrlQuery query(url);
  query.addQueryItem("response_type", "code");
  url.setQuery(query);

  QDesktopServices::openUrl(url);
}
