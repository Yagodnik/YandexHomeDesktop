#pragma once

#include "yh/yandexapiadapter_export.h"

#include <functional>

#include "IHomeApi.h"
#include "IHttpTransport.h"

class YANDEXAPIADAPTER_EXPORT YandexHomeApi final : public QObject, public IHomeApi {
  Q_OBJECT
public:
  using TokenProvider = std::function<QString()>;
  explicit YandexHomeApi(TokenProvider token_provider, IHttpTransport* transport,
                         QObject* parent = nullptr);

  void GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) override;
  void GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) override;
  void GetDeviceInfo(const QString& id, QObject* context,
                     ApiResultHandler<DeviceInfo> handler) override;
  void ExecuteScenario(const QString& id, QObject* context,
                       ApiResultHandler<void> handler) override;
  void PerformActions(const QList<DeviceActionsObject>& actions, QObject* context,
                      ApiResultHandler<void> handler) override;

private:
  const QString kInfoEndpoint = "https://api.iot.yandex.net/v1.0/user/info";
  const QString kExecuteScenarioEndpoint = "https://api.iot.yandex.net/v1.0/scenarios/%1/actions";
  const QString kDeviceInfoEndpoint = "https://api.iot.yandex.net/v1.0/devices/%1";
  const QString kDevicesActionsEndpoint = "https://api.iot.yandex.net/v1.0/devices/actions";

  TokenProvider token_provider_;
  IHttpTransport* transport_;
};
