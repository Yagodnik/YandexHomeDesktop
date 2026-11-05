#pragma once

#include <QPointer>
#include "models/HomeViewModel.h"
#include "models/AccountModel.h"
#include "models/DevicesModel/DevicesFilterModel.h"
#include "utils/Settings.h"

namespace home_test {

class HomeTestAccountApi final : public IAccountApi {
public:
  struct Request {
    QPointer<QObject> context;
    ApiResultHandler<AccountInfo> handler;
    void Reply(ApiResult<AccountInfo> result) const { if (context) { handler(std::move(result)); } }
  };
  QList<Request> requests;
  void LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) override {
    requests.append({context, std::move(handler)});
  }
};

class HomeTestApi final : public IHomeApi {
public:
  struct Request {
    QPointer<QObject> context;
    ApiResultHandler<UserInfo> handler;
    void Reply(ApiResult<UserInfo> result) const { if (context) { handler(std::move(result)); } }
  };
  QList<Request> requests;
  void GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) override {
    requests.append({context, std::move(handler)});
  }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>>) override { QFAIL("Unexpected scenario request"); }
  void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo>) override { QFAIL("Unexpected device request"); }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void>) override { QFAIL("Unexpected execution"); }
  void PerformActions(const QList<DeviceActionsObject>&, QObject*, ApiResultHandler<void>) override { QFAIL("Unexpected action"); }
};

inline UserInfo Snapshot() {
  UserInfo info{};
  info.status = Status::Ok;
  for (const auto& id : {QString("one"), QString("two")}) {
    HouseholdObject household{};
    household.id = id; household.name = "House " + id;
    info.households.append(household);
    RoomObject room{};
    room.id = "room-" + id; room.name = "Room " + id; room.household_id = id;
    info.rooms.append(room);
    DeviceObject device{};
    device.id = "device-" + id; device.name = "Lamp " + id; device.type = "devices.types.light";
    device.room = room.id; device.household_id = id;
    info.devices.append(device);
  }
  return info;
}

}  // namespace home_test
