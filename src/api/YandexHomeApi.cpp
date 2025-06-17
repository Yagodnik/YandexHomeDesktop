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
  : QObject(parent), token_provider_(std::move(token_provider)) {}

void YandexHomeApi::RequestInfo() {
  MakeGetRequest<UserInfo>(kInfoEndpoint, [this](auto& user_info) {
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
  MakeGetRequest<UserInfo>(kInfoEndpoint, [this](auto& user_info) {
    if (user_info.status == Status::Ok) {
      qDebug() << "Received " << user_info.scenarios.length() << " scenarios";

      emit scenariosReceived(user_info.scenarios);
    } else {
      emit errorReceived(user_info.message);
    }
  });
}

void YandexHomeApi::ExecuteScenario(const QString &scenario_id, const std::function<void(const Response&)>& callback) {
  const QString url = kExecuteScenarioEndpoint.arg(scenario_id);

  MakePostRequest<Response>(url, [this, scenario_id, callback](auto& response) {
    if (response.status == Status::Ok) {
      qDebug() << "Scenario" << scenario_id << "executing finished";
      emit scenarioExecutionFinished(scenario_id);
    } else {
      emit errorReceived(response.message);
    }

    callback(response);
  });
}

std::expected<QJsonObject, QString> YandexHomeApi::ParseResponseAsObject(const QByteArray &response) {
  QJsonParseError json_error;
  const QJsonDocument json_response = QJsonDocument::fromJson(
    response, &json_error);

  if (json_error.error != QJsonParseError::NoError) {
    return std::unexpected(json_error.errorString());
  }

  return json_response.object();
}
