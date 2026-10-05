#include "YandexHomeApi.h"

#include <QDebug>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonDocument>
#include <QScopedPointer>
#include <QTimer>

#include "model/Response.h"
#include "RequestFactory.h"
#include "model/Actions.h"

template<Serialization::Serializable T>
void YandexHomeApi::PerformRequest(
  std::function<QNetworkReply*()> send_fn,
  std::function<void(const T&)> ok_callback,
  std::function<void(const QString&)> error_callback
) {
  using ReplyGuard = QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>;

  QNetworkReply *reply = send_fn();

  auto timeout = new QTimer(reply);
  timeout->setSingleShot(true);
  timeout->start(kApiTimeout);

  connect(timeout, &QTimer::timeout, reply, [reply, error_callback]() {
    if (reply->isRunning()) {
      reply->abort();
      error_callback("Timeout reached!");
    }
  });

  connect(reply, &QNetworkReply::finished, [reply, ok_callback, error_callback]() {
    ReplyGuard guard(reply);

    if (reply->error() != QNetworkReply::NoError) {
      error_callback(reply->errorString());
      return;
    }

    const auto response_bytes = reply->readAll();

    qDebug() << "";
    qDebug() << response_bytes.toStdString();
    qDebug() << "";

    const auto response_object = ParseResponseAsObject(response_bytes);

    if (!response_object.has_value()) {
      error_callback(response_object.error());
      return;
    }

    const auto response = Serialization::From<T>(response_object.value());
    ok_callback(response);
  });
}

template<Serialization::Serializable T>
void YandexHomeApi::MakeGetRequest(
  const QString &endpoint,
  std::function<void(const T&)> ok_callback,
  std::function<void(const QString&)> error_callback
) {
  const auto request = RequestFactory::CreateBearer(endpoint, token_provider_());

  PerformRequest<T>(
    [this, &request]() { return network_access_manager_.get(request); },
    std::move(ok_callback),
    std::move(error_callback)
  );
}

template<Serialization::Serializable T>
void YandexHomeApi::MakePostRequest(
  const QString &endpoint,
  std::function<void(const T&)> ok_callback,
  std::function<void(const QString&)> error_callback
) {
  const auto request = RequestFactory::CreateBearer(endpoint, token_provider_());

  PerformRequest<T>(
    [this, &request]() { return network_access_manager_.post(request, nullptr); },
    std::move(ok_callback),
    std::move(error_callback)
  );
}

template<Serialization::Serializable T>
void YandexHomeApi::MakePostRequest(
  const QString &endpoint,
  std::function<void(const T&)> ok_callback,
  std::function<void(const QString&)> error_callback,
  const QByteArray& data
) {
  const auto request = RequestFactory::CreateBearer(endpoint, token_provider_());

  PerformRequest<T>(
    [this, &request, data]() { return network_access_manager_.post(request, data); },
    std::move(ok_callback),
    std::move(error_callback)
  );
}

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
    qCritical() << "YandexHomeApi: Internal error: " << message;

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
      qCritical() << "YandexHomeApi: Error getting during getting scenarios";

      emit scenariosReceivingFailed(user_info.message);
    }
  };

  auto error_callback = [this](const QString& message) {
    qCritical() << "YandexHomeApi: Internal error: " << message;

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
      qInfo() << "YandexHomeApi: Received device info";

      emit deviceInfoReceived(info);
    } else {
      qCritical() << "YandexHomeApi: Error getting during getting scenarios";

      emit deviceInfoReceivingFailed(info.message);
    }
  };

  auto error_callback = [this](const QString& message) {
    qCritical() << "YandexHomeApi: Internal error: " << message;

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
      qInfo() << "YandexHomeApi: Scenario" << scenario_id << "executing finished";
      emit scenarioExecutionFinishedSuccessfully(scenario_id, user_data);
    } else {
      emit scenarioExecutionFailed(response.message, user_data);
    }
  };

  auto error_callback = [this, user_data](const QString& message) {
    qCritical() << "YandexHomeApi: Internal error: " << message;
    emit scenarioExecutionFailed(message, user_data);
  };

  MakePostRequest<Response>(
    url,
    ok_callback,
    error_callback
  );
}

void YandexHomeApi::PerformActions(const QList<DeviceActionsObject> &actions, const QVariant& user_data) {
  auto ok_callback = [this, user_data](const DeviceActionResponse& response) {
    if (response.status == Status::Error) {
      qCritical() << "YandexHomeApi: Action " << response.request_id << "executing failed";

      emit actionExecutingFailed(response.message, user_data);
      return;
    }

    qInfo() << "YandexHomeApi: Action " << response.request_id << "executing finished";

    for (const auto& device : response.devices) {
      for (const auto& capability : device.capabilities) {
        const auto& state = capability.state;
        const auto& action_result = state.action_result;

        if (action_result.status == "DONE") {
          emit actionExecutingFinishedSuccessfully(user_data);
        } else {
          qCritical() << "YandexHomeApi: Action " << response.request_id << "executing failed with:";
          qCritical() << "YandexHomeApi: Code:" << action_result.error_code;
          qCritical() << "YandexHomeApi: Message:" << action_result.error_message;
          emit actionExecutingFailed(action_result.error_code, user_data);
        }
      }
    }
  };

  auto error_callback = [this, user_data](const QString& message) {
    qCritical() << "YandexHomeApi: Internal error: " << message;
    emit actionExecutingFailed(message, user_data);
  };

  QJsonArray json_actions;
  for (const auto& action : actions) {
    json_actions.push_back(Serialization::To<DeviceActionsObject>(action));
  }

  QJsonObject json_payload;
  json_payload["devices"] = json_actions;

  const QByteArray payload = QJsonDocument(json_payload).toJson();

  MakePostRequest<DeviceActionResponse>(
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
