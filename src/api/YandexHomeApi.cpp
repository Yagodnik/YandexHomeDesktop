#include "YandexHomeApi.h"

#include <qcommandlineparser.h>
#include <QDebug>
#include <QNetworkReply>

#include "model/Response.h"
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
  MakeApiRequest<UserInfo>(kInfoEndpoint, [this](auto& user_info) {
    if (user_info.status == Status::Ok) {
      emit infoReceived(user_info);
    } else {
      emit errorReceived(user_info.message);
    }

    qDebug() << "User info:";
    qDebug() << "Status:" << (user_info.status == Status::Ok ? "Ok" : "Error");
    qDebug() << "Request ID:" << user_info.request_id;
  });
}

void YandexHomeApi::GetScenarios() {
  MakeApiRequest<UserInfo>(kInfoEndpoint, [this](auto& user_info) {
    if (user_info.status == Status::Ok) {
      qDebug() << "Received " << user_info.scenarios.length() << " scenarios";

      emit scenariosReceived(user_info.scenarios);
    } else {
      emit errorReceived(user_info.message);
    }
  });
}

void YandexHomeApi::ExecuteScenario(const QString &scenario_id) {
  QString url = QStringLiteral("https://api.iot.yandex.net/v1.0/scenarios/%1/actions").arg(scenario_id);

  const auto request = RequestFactory::Create(url, token_provider_());
  QNetworkReply *reply = network_access_manager_.post(request, nullptr);
}
