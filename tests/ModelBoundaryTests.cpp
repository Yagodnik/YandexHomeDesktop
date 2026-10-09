#include "ModelBoundaryTests.h"

#include <QPointer>
#include <QtTest>
#include <optional>

#include "api/IAccountApi.h"
#include "api/IHomeApi.h"
#include "models/AccountModel.h"
#include "models/DeviceModel/DeviceController.h"
#include "models/DeviceModel/DeviceDataModel.h"
#include "models/DeviceModel/DeviceViewModel.h"
#include "models/DevicesModel/DevicesModel.h"
#include "models/HomeViewModel.h"
#include "models/HouseholdsModel/HouseholdsModel.h"
#include "models/RoomsModel/RoomsModel.h"
#include "models/ScenariosModel/ScenariosViewModel.h"

namespace {
template<typename T>
struct PendingResult {
  QPointer<QObject> context;
  ApiResultHandler<T> handler;

  void Send(ApiResult<T> result) {
    if (context) {
      handler(std::move(result));
    }
  }
};

class FakeHomeApi final : public IHomeApi {
public:
  std::optional<ApiResult<DeviceInfo>> immediate_device_result;
  QList<PendingResult<UserInfo>> user_requests;
  QList<PendingResult<QList<ScenarioObject>>> scenario_requests;
  QList<PendingResult<DeviceInfo>> device_requests;
  QList<PendingResult<void>> execution_requests;
  QList<PendingResult<void>> action_requests;

  void GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) override {
    user_requests.append({context, std::move(handler)});
  }
  void GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) override {
    scenario_requests.append({context, std::move(handler)});
  }
  void GetDeviceInfo(const QString&, QObject* context, ApiResultHandler<DeviceInfo> handler) override {
    if (immediate_device_result) {
      handler(*immediate_device_result);
      return;
    }
    device_requests.append({context, std::move(handler)});
  }
  void ExecuteScenario(const QString&, QObject* context, ApiResultHandler<void> handler) override {
    execution_requests.append({context, std::move(handler)});
  }
  void PerformActions(const QList<DeviceActionsObject>&, QObject* context,
                      ApiResultHandler<void> handler) override {
    action_requests.append({context, std::move(handler)});
  }
};

class FakeAccountApi final : public IAccountApi {
public:
  PendingResult<AccountInfo> request;
  void LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) override {
    request = {context, std::move(handler)};
  }
};
}

void ModelBoundaryTests::HomeRefreshUpdatesAllModels() {
  FakeHomeApi api;
  HomeService service(&api);
  HomeViewModel view_model(&service);
  auto& devices = *view_model.GetDevices();
  auto& rooms = *view_model.GetRooms();
  auto& households = *view_model.GetHouseholds();
  QSignalSpy loaded(&service, &HomeService::snapshotChanged);

  view_model.EnsureLoaded();
  QCOMPARE(api.user_requests.size(), 1);

  UserInfo info;
  info.status = Status::Ok;
  DeviceObject device;
  device.id = "device-1";
  device.name = "Lamp";
  info.devices.append(device);
  RoomObject room;
  room.id = "room-1";
  room.name = "Office";
  room.household_id = "house-1";
  info.rooms.append(room);
  HouseholdObject household;
  household.id = "house-1";
  household.name = "Home";
  info.households.append(household);

  api.user_requests[0].Send(info);
  QCOMPARE(devices.rowCount({}), 1);
  QCOMPARE(rooms.rowCount({}), 1);
  QCOMPARE(households.rowCount({}), 1);
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(view_model.GetCurrentHousehold(), QString("house-1"));

  view_model.Refresh();
  QCOMPARE(api.user_requests.size(), 2);
  auto failure = std::unexpected(ApiError{ApiErrorKind::Timeout, "Timeout reached!"});
  api.user_requests[1].Send(failure);
  QCOMPARE(view_model.GetState(), HomeViewModel::Error);

}

void ModelBoundaryTests::ScenarioResultsRemainScoped() {
  FakeHomeApi api;
  ScenarioService service(&api);
  ScenariosViewModel view_model(&service);
  auto& model = *view_model.GetScenarios();
  QSignalSpy loaded(&service, &ScenarioService::scenariosChanged);
  QSignalSpy failed(&view_model, &ScenariosViewModel::executionFailed);
  view_model.EnsureLoaded();
  QCOMPARE(api.scenario_requests.size(), 1);

  ScenarioObject scenario;
  scenario.id = "s1";
  scenario.name = "Evening";
  scenario.is_active = true;
  api.scenario_requests[0].Send(QList<ScenarioObject>{scenario});
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(model.rowCount({}), 1);

  view_model.ExecuteScenario("s1");
  QCOMPARE(api.execution_requests.size(), 1);
  QCOMPARE(model.data(model.index(0), ScenariosModel::IsWaitingResponseRole).toBool(), true);
  api.execution_requests[0].Send(std::unexpected(ApiError{ApiErrorKind::Service, "failed"}));
  QCOMPARE(failed.size(), 1);
  QCOMPARE(model.data(model.index(0), ScenariosModel::IsWaitingResponseRole).toBool(), false);
}

void ModelBoundaryTests::DeviceDataFlowsThroughController() {
  FakeHomeApi api;
  DeviceController controller(&api);
  DeviceDataModel model(&controller);
  QSignalSpy initialized(&model, &DeviceDataModel::initialized);

  controller.LoadDevice("device-1");
  QCOMPARE(api.device_requests.size(), 1);
  DeviceInfo info;
  info.status = Status::Ok;
  info.id = "device-1";
  info.name = "Lamp";
  info.state = DeviceState::Online;
  api.device_requests[0].Send(info);
  QCOMPARE(model.GetDeviceName(), QString("Lamp"));
  QVERIFY(model.IsDeviceOnline());
  QCOMPARE(initialized.size(), 1);
  controller.StopPolling();
}

void ModelBoundaryTests::ImmediateDeviceResultSurvivesReset() {
  FakeHomeApi api;
  DeviceInfo info;
  info.status = Status::Ok;
  info.id = "device-1";
  info.name = "Lamp";
  info.state = DeviceState::Online;
  api.immediate_device_result = info;

  DeviceController controller(&api);
  DeviceDataModel model(&controller);
  QSignalSpy initialized(&model, &DeviceDataModel::initialized);

  controller.LoadDevice("device-1");
  QCOMPARE(model.GetDeviceName(), QString("Lamp"));
  QVERIFY(model.IsDeviceOnline());
  QCOMPARE(initialized.size(), 1);
  controller.StopPolling();
}

void ModelBoundaryTests::ActionEventsReachController() {
  FakeHomeApi api;
  DeviceController controller(&api);
  QSignalSpy errors(&controller, &DeviceController::errorOccurred);

  CapabilityObject capability;
  capability.type = CapabilityType::OnOff;
  capability.state = {{"instance", "on"}, {"value", false}};
  controller.LoadDevice("device-1");
  DeviceInfo info;
  info.status = Status::Ok;
  info.id = "device-1";
  info.capabilities = {capability};
  api.device_requests[0].Send(info);

  controller.UseCapability(0, capability, {{"instance", "on"}, {"value", true}});
  QCOMPARE(api.action_requests.size(), 1);
  auto failure = std::unexpected(ApiError{ApiErrorKind::Service, "FAILED"});
  api.action_requests[0].Send(failure);
  QCOMPARE(errors.size(), 1);
  controller.StopPolling();
}

void ModelBoundaryTests::AccountModelKeepsQmlContract() {
  FakeAccountApi api;
  AccountModel model(&api);
  QSignalSpy loaded(&model, &AccountModel::dataLoaded);
  QSignalSpy failed(&model, &AccountModel::dataLoadingFailed);

  model.LoadData();
  api.request.Send(AccountInfo{"Ada", "avatar-1", "ada@example.com"});
  QCOMPARE(loaded.size(), 1);
  QCOMPARE(model.GetName(), QString("Ada"));
  QCOMPARE(model.GetAvatarUrl(), QString("https://avatars.yandex.net/get-yapic/avatar-1/"));
  QCOMPARE(model.GetEmail(), QString("ada@example.com"));

  model.LoadData();
  api.request.Send(std::unexpected(ApiError{ApiErrorKind::Network, "offline"}));
  QCOMPARE(failed.size(), 1);
}

void ModelBoundaryTests::AccountResetRejectsOldSessionResults() {
  FakeAccountApi api;
  AccountModel model(&api);
  QSignalSpy loaded(&model, &AccountModel::dataLoaded);
  QSignalSpy failed(&model, &AccountModel::dataLoadingFailed);
  model.LoadData();
  api.request.Send(AccountInfo{"Ada", "avatar", "ada@example.com"});
  model.LoadData();
  auto stale = api.request;
  model.Reset();
  QVERIFY(model.GetName().isEmpty());
  QVERIFY(model.GetEmail().isEmpty());
  QCOMPARE(model.GetAvatarUrl(), QString("qrc:/images/icon.png"));
  const auto notifications = loaded.size();
  stale.Send(AccountInfo{"Old session", "old-avatar", "old@example.com"});
  stale.Send(std::unexpected(ApiError{ApiErrorKind::Network, "late error"}));
  QCOMPARE(loaded.size(), notifications);
  QCOMPARE(failed.size(), 0);
  QVERIFY(model.GetName().isEmpty());
  model.LoadData();
  api.request.Send(AccountInfo{"New session", "", "new@example.com"});
  QCOMPARE(model.GetName(), QString("New session"));
}

void ModelBoundaryTests::DeviceResetRejectsOldSessionResults() {
  FakeHomeApi api;
  DeviceService service(&api);
  DeviceViewModel model(&service, nullptr, [] { return 100.; });
  QSignalSpy received(&model, &DeviceViewModel::deviceInfoReceived);
  QSignalSpy errors(&model, &DeviceViewModel::errorOccurred);
  model.LoadDevice("device");
  DeviceInfo info;
  info.status = Status::Ok;
  info.id = "device";
  info.name = "Old session";
  api.device_requests.first().Send(info);
  QCOMPARE(model.GetState(), DeviceViewModel::Ready);
  model.Refresh();
  auto stale_read = api.device_requests.last();
  model.ResetSession();
  QCOMPARE(model.GetState(), DeviceViewModel::Loading);
  QCOMPARE(model.GetCapabilities()->rowCount(), 0);
  QCOMPARE(model.GetProperties()->rowCount(), 0);
  const auto notifications = received.size();
  model.LoadDevice("device");
  stale_read.Send(info);
  stale_read.Send(std::unexpected(ApiError{ApiErrorKind::Network, "old session error"}));
  QCOMPARE(received.size(), notifications);
  QCOMPARE(errors.size(), 0);
  QCOMPARE(model.GetState(), DeviceViewModel::Loading);
  info.name = "New session";
  api.device_requests.last().Send(info);
  QCOMPARE(model.GetState(), DeviceViewModel::Ready);
  model.StopPolling();
}
