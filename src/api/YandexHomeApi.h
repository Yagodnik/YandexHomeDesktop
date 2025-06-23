#pragma once

#include <expected>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include "RequestFactory.h"
#include "model/Actions.h"
#include "model/Response.h"
#include "model/UserInfo.h"

class YandexHomeApi final : public QObject {
  Q_OBJECT
public:
  using TokenProvider = std::function<QString()>;
  explicit YandexHomeApi(TokenProvider token_provider, QObject *parent = nullptr);

  Q_INVOKABLE void GetUserInfo();
  Q_INVOKABLE void GetScenarios();
  Q_INVOKABLE void GetDeviceInfo(const QString& id);

  void ExecuteScenario(const QString& scenario_id, const QVariant& user_data = QVariant());
  void PerformActions(const QList<DeviceActionsObject>& actions, const QVariant& user_data = QVariant());

signals:
  void userInfoReceived(const UserInfo& info);
  void userInfoReceivingFailed(const QString& message);

  void scenariosReceivedSuccessfully(const QList<ScenarioObject>& scenarios);
  void scenariosReceivingFailed(const QString& message);

  void deviceInfoReceived(const DeviceObject& info);
  void deviceInfoReceivingFailed(const QString& message);

  void errorReceived(const QString& error);

  void scenarioExecutionFinished(const QString& scenario_id);
  void scenarioExecutionFinishedSuccessfully(const QString& scenario_id, const QVariant& user_data);
  void scenarioExecutionFailed(const QString& scenario_id, const QVariant& user_data);

  void actionExecutingFinishedSuccessfully(const QVariant& user_data);
  void actionExecutingFailed(const QString& message, const QVariant& user_data);

private:
  using ReplyGuard = QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>;

  static constexpr int kApiTimeout = 5000;
  const QString kInfoEndpoint = "https://api.iot.yandex.net/v1.0/user/info";
  const QString kExecuteScenarioEndpoint = "https://api.iot.yandex.net/v1.0/scenarios/%1/actions";
  const QString kUseCapabilityEndpoint = "https://api.iot.yandex.net/v1.0/devices/actions";
  const QString kDeviceInfoEndpoint = "https://api.iot.yandex.net/v1.0/devices/%1";
  const QString kDevicesActionsEndpoint = "https://api.iot.yandex.net/v1.0/devices/actions";

  static std::expected<QJsonObject, QString> ParseResponseAsObject(const QByteArray& response);

  template<Serialization::Serializable T>
  static void PerformRequest(
    std::function<QNetworkReply*()> send_fn,
    std::function<void(const T&)> ok_callback,
    std::function<void(const QString&)> error_callback
  ) {
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
  void MakeGetRequest(
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
  void MakePostRequest(
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
  void MakePostRequest(
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

  QNetworkAccessManager network_access_manager_;
  TokenProvider token_provider_;
};
