#include "YandexHomeApi.h"

#include <qcommandlineparser.h>
#include <QDebug>
#include <QNetworkReply>

#include "qtkeychain/include/qtkeychain/keychain.h"

QNetworkRequest RequestFactory::Create(const QString &endpoint, const QString &token) {
  QNetworkRequest request(endpoint);
  const QString token_string = kBearer + token;
  const QByteArray token_bytes = token_string.toUtf8();

  request.setRawHeader("User-Agent", "YandexHomeApi");
  request.setRawHeader("Authorization", token_bytes);

  return request;
}

YandexHomeApi::YandexHomeApi(TokenProvider token_provider, QObject *parent)
  : QObject(parent), token_provider_(std::move(token_provider))
{}

void YandexHomeApi::RequestInfo() {
  const auto request = RequestFactory::Create(kInfoEndpoint, token_provider_());

  QNetworkReply *reply = network_access_manager_.get(request);

  connect(reply, &QNetworkReply::finished, [reply, this]() {
    ReplyGuard guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << "YandexHomeApi::RequestInfo Error:" << reply->errorString();

      emit errorReceived(reply->errorString());

      return;
    }

    const QByteArray response_bytes = reply->readAll();

    QJsonParseError json_error;
    const QJsonDocument json_response = QJsonDocument::fromJson(
      response_bytes, &json_error);

    if (json_error.error != QJsonParseError::NoError) {
      qWarning() << "Failed to parse JSON:" << json_error.errorString();
      emit errorReceived(json_error.errorString());
      return;
    }

    const QJsonObject response_object = json_response.object();

    const auto response = Serialization::From<UserInfo>(response_object);

    if (response.status == Status::Ok) {
      emit infoReceived(response);
    } else {
      emit errorReceived(response.message);
    }

    qDebug() << "User info:";

    qDebug() << "Status:" << (response.status == Status::Ok ? "Ok" : "Error");
    qDebug() << "Request ID:" << response.request_id;
  });
}
