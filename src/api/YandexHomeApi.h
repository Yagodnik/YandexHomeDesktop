#pragma once

#include <expected>
#include <functional>
#include <QObject>
#include <QNetworkAccessManager>

#include "model/Actions.h"
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

  void deviceInfoReceived(const DeviceInfo &info);
  void deviceInfoReceivingFailed(const QString& message);

  void errorReceived(const QString& error);

  void scenarioExecutionFinished(const QString& scenario_id);
  void scenarioExecutionFinishedSuccessfully(const QString& scenario_id, const QVariant& user_data);
  void scenarioExecutionFailed(const QString& scenario_id, const QVariant& user_data);

  void actionExecutingFinishedSuccessfully(const QVariant& user_data);
  void actionExecutingFailed(const QString& message, const QVariant& user_data);

private:
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
  );

  template<Serialization::Serializable T>
  void MakeGetRequest(
    const QString &endpoint,
    std::function<void(const T&)> ok_callback,
    std::function<void(const QString&)> error_callback
  );

  template<Serialization::Serializable T>
  void MakePostRequest(
    const QString &endpoint,
    std::function<void(const T&)> ok_callback,
    std::function<void(const QString&)> error_callback
  );

  template<Serialization::Serializable T>
  void MakePostRequest(
    const QString &endpoint,
    std::function<void(const T&)> ok_callback,
    std::function<void(const QString&)> error_callback,
    const QByteArray& data
  );

  QNetworkAccessManager network_access_manager_;
  TokenProvider token_provider_;
};
