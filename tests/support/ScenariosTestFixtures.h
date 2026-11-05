#pragma once

#include <QPointer>
#include "api/IHomeApi.h"
#include "models/ScenariosModel/ScenariosViewModel.h"

namespace scenarios_test {

template<typename T>
struct PendingResult {
  QPointer<QObject> context;
  ApiResultHandler<T> handler;
  QString scenario_id;

  void Send(ApiResult<T> result) const {
    if (context) {
      handler(std::move(result));
    }
  }
};

class FakeHomeApi : public IHomeApi {
public:
  QList<PendingResult<QList<ScenarioObject>>> loads;
  QList<PendingResult<void>> executions;

  void GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) override {
    loads.append({context, std::move(handler), {}});
  }
  void ExecuteScenario(const QString& id, QObject* context, ApiResultHandler<void> handler) override {
    executions.append({context, std::move(handler), id});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo>) override { QFAIL("Unexpected user-info request"); }
  void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo>) override { QFAIL("Unexpected device request"); }
  void PerformActions(const QList<DeviceActionsObject>&, QObject*, ApiResultHandler<void>) override { QFAIL("Unexpected action request"); }
};

inline ScenarioObject Scenario(const QString& id, bool active = true) {
  ScenarioObject scenario;
  scenario.id = id;
  scenario.name = "Scenario " + id;
  scenario.is_active = active;
  return scenario;
}

}  // namespace scenarios_test
