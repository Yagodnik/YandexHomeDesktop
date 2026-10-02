#pragma once

#include <QObject>

#include "ApiResult.h"
#include "model/Actions.h"
#include "model/UserInfo.h"

class IHomeApi {
public:
  virtual ~IHomeApi() = default;

  // The context owns delivery: handlers must not run after it is destroyed.
  virtual void GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) = 0;
  virtual void GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) = 0;
  virtual void GetDeviceInfo(const QString& id, QObject* context, ApiResultHandler<DeviceInfo> handler) = 0;
  virtual void ExecuteScenario(const QString& id, QObject* context, ApiResultHandler<void> handler) = 0;
  virtual void PerformActions(const QList<DeviceActionsObject>& actions, QObject* context,
                              ApiResultHandler<void> handler) = 0;
};
