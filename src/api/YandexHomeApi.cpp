#include "YandexHomeApi.h"

#include <QDebug>
#include <QNetworkReply>

#include "model/Response.h"
#include "RequestFactory.h"
#include "model/Actions.h"

YandexHomeApi::YandexHomeApi(TokenProvider token_provider, QObject *parent)
  : QObject(parent), token_provider_(std::move(token_provider)) {}

void YandexHomeApi::GetUserInfo() {
  auto ok_callback = [this](auto& user_info) {
    if (user_info.status == Status::Ok) {
      emit userInfoReceived(user_info);
    } else {
      emit userInfoReceivingFailed(user_info.message);
    }

    qDebug() << "User info:";
    qDebug() << "Status:" << (user_info.status == Status::Ok ? "Ok" : "Error");
    qDebug() << "Request ID:" << user_info.request_id;
  };

  auto error_callback = [this](const QString& message) {
    qDebug() << "YandexHomeApi: Interal error: " << message;

    emit userInfoReceivingFailed(message);
  };

  MakeGetRequest<UserInfo>(
    kInfoEndpoint,
    ok_callback,
    error_callback
  );
}

void YandexHomeApi::GetScenarios() {
  auto ok_callback = [this](auto& user_info) {
    if (user_info.status == Status::Ok) {
      qDebug() << "YandexHomeApi: Received " << user_info.scenarios.length() << " scenarios";

      emit scenariosReceivedSuccessfully(user_info.scenarios);
    } else {
      qDebug() << "YandexHomeApi: Error getting during getting scenarios";

      emit scenariosReceivingFailed(user_info.message);
    }
  };

  auto error_callback = [this](const QString& message) {
    qDebug() << "YandexHomeApi: Interal error: " << message;

    emit scenariosReceivingFailed(message);
  };

  MakeGetRequest<UserInfo>(
    kInfoEndpoint,
    ok_callback,
    error_callback
  );
}

void YandexHomeApi::GetDeviceInfo(const QString &id) {
  const auto url = kDeviceInfoEndpoint.arg(id);

  auto ok_callback = [this](auto& info) {
    if (info.status == Status::Ok) {
      qDebug() << "YandexHomeApi: Received device info";

      emit deviceInfoReceived({
        .capabilities = info.capabilities
      });
    } else {
      qDebug() << "YandexHomeApi: Error getting during getting scenarios";

      emit deviceInfoReceivingFailed(info.message);
    }
  };

  auto error_callback = [this](const QString& message) {
    qDebug() << "YandexHomeApi: Interal error: " << message;

    emit deviceInfoReceivingFailed(message);
  };

  MakeGetRequest<DeviceInfo>(
    url,
    ok_callback,
    error_callback
  );
}

void YandexHomeApi::ExecuteScenario(const QString &scenario_id, const QVariant& user_data) {
  const QString url = kExecuteScenarioEndpoint.arg(scenario_id);

  auto ok_callback = [this, scenario_id, user_data](auto& response) {
    if (response.status == Status::Ok) {
      qDebug() << "Scenario" << scenario_id << "executing finished";
      emit scenarioExecutionFinishedSuccessfully(scenario_id, user_data);
    } else {
      emit scenarioExecutionFailed(response.message, user_data);
    }
  };

  auto error_callback = [this, user_data](const QString& message) {
    qDebug() << "YandexHomeApi: Internal error: " << message;
    emit scenarioExecutionFailed(message, user_data);
  };

  MakePostRequest<Response>(
    url,
    ok_callback,
    error_callback
  );
}

void YandexHomeApi::PerformActions(const QList<DeviceActionsObject> &actions) {
  auto ok_callback = [this](auto& response) {
    if (response.status == Status::Ok) {
      qDebug() << "Action " << response.request_id << "executing finished";
    } else {
      qDebug() << "Action " << response.request_id << "executing failed";
      qDebug() << response.message;
    }
  };

  auto error_callback = [this](const QString& message) {
    qDebug() << "YandexHomeApi: Internal error: " << message;
  };

  QJsonArray json_actions;
  for (const auto& action : actions) {
    json_actions.push_back(Serialization::To<DeviceActionsObject>(action));
  }

  QJsonObject json_payload;
  json_payload["devices"] = json_actions;

  const QByteArray payload = QJsonDocument(json_payload).toJson();

  MakePostRequest<Response>(
    kDevicesActionsEndpoint,
    ok_callback,
    error_callback,
    payload
  );
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
