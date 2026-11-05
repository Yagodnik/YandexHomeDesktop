#include <QPointer>
#include <QJsonArray>
#include <QTest>
#include "mcp/McpBridge.h"
#include "services/HomeService.h"
#include "services/DeviceService.h"
#include "services/ScenarioService.h"

class FakeApi final : public IHomeApi {
public:
  QPointer<QObject> context;
  ApiResultHandler<UserInfo> home;
  int device_reads = 0;
  QString scenario_run;
  QList<DeviceActionsObject> actions;
  void GetUserInfo(QObject* owner, ApiResultHandler<UserInfo> handler) override { context = owner; home = std::move(handler); }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>> handler) override {
    ScenarioObject scenario{}; scenario.id = "morning"; scenario.name = "Morning"; scenario.is_active = true;
    handler(QList<ScenarioObject>{scenario});
  }
  void GetDeviceInfo(const QString& id, QObject*, ApiResultHandler<DeviceInfo> handler) override {
    ++device_reads;
    DeviceInfo device{}; device.id = id; device.name = "Lamp"; device.status = Status::Ok;
    CapabilityObject capability{}; capability.type = CapabilityType::OnOff;
    capability.state = {{"instance", "on"}, {"value", false}};
    device.capabilities.append(capability);
    handler(device);
  }
  void ExecuteScenario(const QString& id, QObject*, ApiResultHandler<void> handler) override {
    scenario_run = id; handler(ApiResult<void>{});
  }
  void PerformActions(const QList<DeviceActionsObject>& value, QObject*, ApiResultHandler<void> handler) override {
    actions = value; handler(ApiResult<void>{});
  }
};

class McpBridgeTests final : public QObject {
  Q_OBJECT
private slots:
  void AsyncRequestsAndReset() {
    FakeApi api;
    HomeService home(&api); DeviceService devices(&api); ScenarioService scenarios(&api);
    McpBridge bridge(&home, &devices, &scenarios);
    const auto submitted = bridge.Handle({{"command", "submit"}, {"tool", "list_devices"}, {"arguments", QJsonObject{}}});
    QVERIFY(submitted["ok"].toBool());
    const auto id = submitted["id"].toString();
    QVERIFY(bridge.Handle({{"command", "poll"}, {"id", id}})["pending"].toBool());
    UserInfo data{}; data.status = Status::Ok;
    DeviceObject device{}; device.id = "lamp"; device.name = "Lamp"; data.devices.append(device);
    api.home(data);
    const auto response = bridge.Handle({{"command", "poll"}, {"id", id}});
    QVERIFY(response["ok"].toBool());
    QVERIFY(!response["pending"].toBool());
    QCOMPARE(response["result"].toObject()["devices"].toArray()[0].toObject()["id"].toString(), "lamp");
    QVERIFY(!bridge.Handle({{"command", "poll"}, {"id", id}})["ok"].toBool());
    bridge.Handle({{"command", "submit"}, {"tool", "list_devices"}, {"arguments", QJsonObject{}}});
    QVERIFY(api.context);
    bridge.Reset();
    QVERIFY(!api.context);
  }
  void DispatchCommandsAndInlineCompletion() {
    FakeApi api;
    HomeService home(&api); DeviceService devices(&api); ScenarioService scenarios(&api);
    McpBridge bridge(&home, &devices, &scenarios);
    const auto call = [&bridge](const QString& name, const QJsonObject& args) {
      const auto submitted = bridge.Handle({{"command", "submit"}, {"tool", name}, {"arguments", args}});
      return bridge.Handle({{"command", "poll"}, {"id", submitted["id"]}})["result"].toObject();
    };
    const auto device = call("get_device", {{"device_id", "lamp"}});
    QCOMPARE(device["device"].toObject()["id"].toString(), "lamp");
    QCOMPARE(api.device_reads, 1);
    const auto scenarios_result = call("list_scenarios", {});
    QCOMPARE(scenarios_result["scenarios"].toArray()[0].toObject()["id"].toString(), "morning");
    QVERIFY(call("run_scenario", {{"scenario_id", "morning"}})["ok"].toBool());
    QCOMPARE(api.scenario_run, "morning");
    QVERIFY(!call("control_device", {{"device_id", "lamp"}, {"capability", "on_off"},
                                     {"state", QJsonObject{{"instance", "on"}, {"value", "invalid"}}}})["ok"].toBool());
    QCOMPARE(api.device_reads, 1);
    QVERIFY(!bridge.Handle({{"command", "submit"}, {"tool", "unknown"}, {"arguments", QJsonObject{}}})["ok"].toBool());
    QVERIFY(!bridge.Handle({{"command", "submit"}, {"tool", "control_device"},
      {"arguments", QJsonObject{{"device_id", "lamp"}, {"capability", "on_off"},
         {"state", QJsonObject{{"instance", "on"}, {"value", true}, {"extra", true}}}}}})["ok"].toBool());
    QCOMPARE(api.device_reads, 1);
  }
  void DiscoverRoomAndTurnOnLamp() {
    FakeApi api;
    HomeService home(&api); DeviceService devices(&api); ScenarioService scenarios(&api);
    McpBridge bridge(&home, &devices, &scenarios);
    const auto tools = bridge.Handle({{"command", "tools"}})["tools"].toArray();
    QCOMPARE(tools.size(), 5);
    bool found = false;
    for (const auto& value : tools) {
      const auto tool = value.toObject();
      if (tool["name"] == "list_devices") {
        found = tool["description"].toString().contains("room");
      }
    }
    QVERIFY(found);
    const auto submitted = bridge.Handle({{"command", "submit"}, {"tool", "list_devices"}, {"arguments", QJsonObject{}}});
    UserInfo data{}; data.status = Status::Ok;
    RoomObject room{}; room.id = "living"; room.name = "Living room"; data.rooms.append(room);
    DeviceObject lamp{}; lamp.id = "lamp-1"; lamp.name = "Lamp"; lamp.room = room.id; lamp.aliases = {"Ceiling light"}; data.devices.append(lamp);
    api.home(data);
    const auto home_result = bridge.Handle({{"command", "poll"}, {"id", submitted["id"]}})["result"].toObject();
    QCOMPARE(home_result["rooms"].toArray()[0].toObject()["name"].toString(), "Living room");
    const auto found_lamp = home_result["devices"].toArray()[0].toObject();
    QCOMPARE(found_lamp["room"].toString(), "living");
    QCOMPARE(found_lamp["aliases"].toArray()[0].toString(), "Ceiling light");
    const auto action = bridge.Handle({{"command", "submit"}, {"tool", "control_device"},
      {"arguments", QJsonObject{{"device_id", found_lamp["id"]}, {"capability", "on_off"},
                                {"state", QJsonObject{{"instance", "on"}, {"value", true}}}}}});
    QVERIFY(bridge.Handle({{"command", "poll"}, {"id", action["id"]}})["result"].toObject()["ok"].toBool());
    QCOMPARE(api.actions.size(), 1);
    QCOMPARE(api.actions[0].id, "lamp-1");
    QCOMPARE(api.actions[0].actions[0].state["value"].toBool(), true);
  }
};
QTEST_GUILESS_MAIN(McpBridgeTests)
#include "McpBridgeTests.moc"
