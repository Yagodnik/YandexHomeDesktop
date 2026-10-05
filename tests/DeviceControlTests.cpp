#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include "api/IHomeApi.h"
#include "models/DeviceModel/CapabilitiesModel.h"
#include "models/DeviceModel/DeviceDataModel.h"
#include "models/DeviceModel/PropertiesModel.h"

namespace {
template<typename T>
struct PendingReply {
  QPointer<QObject> context;
  ApiResultHandler<T> handler;

  bool Reply(ApiResult<T> result) {
    if (!context || !handler) {
      return false;
    }
    auto callback = std::move(handler);
    callback(std::move(result));
    return true;
  }
};

// A read captures the server state when requested. Tests independently control
// server writes and callback delivery, including acknowledgements arriving late.
class ScriptedHomeApi final : public IHomeApi {
public:
  struct DeviceRequest : PendingReply<DeviceInfo> {
    QString device_id;
    DeviceInfo snapshot;
  };
  struct ActionRequest : PendingReply<void> {
    QList<DeviceActionsObject> actions;
  };

  DeviceInfo remote{};
  QList<DeviceRequest> device_requests;
  QList<ActionRequest> action_requests;

  void GetDeviceInfo(const QString& id, QObject* context,
                     ApiResultHandler<DeviceInfo> handler) override {
    device_requests.append({{context, std::move(handler)}, id, remote});
  }
  void PerformActions(const QList<DeviceActionsObject>& actions, QObject* context,
                      ApiResultHandler<void> handler) override {
    action_requests.append({{context, std::move(handler)}, actions});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo>) override {
    QFAIL("Unexpected user-info request");
  }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>>) override {
    QFAIL("Unexpected scenarios request");
  }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void>) override {
    QFAIL("Unexpected scenario execution");
  }
};

CapabilityObject Capability(CapabilityType type, const QVariantMap& state,
                            const QVariantMap& parameters = {}) {
  CapabilityObject capability{};
  capability.type = type;
  capability.retrievable = true;
  capability.state = state;
  capability.parameters = parameters;
  capability.last_updated = 100;
  return capability;
}

DeviceInfo InitialDevice() {
  DeviceInfo device{};
  device.status = Status::Ok;
  device.id = "lamp-1";
  device.name = "Lamp";
  device.state = DeviceState::Online;
  device.capabilities = {
    Capability(CapabilityType::Range, {{"instance", "brightness"}, {"value", 20}},
      {{"instance", "brightness"}, {"range", QVariantMap{{"min", 0}, {"max", 100}, {"precision", 1}}}}),
    Capability(CapabilityType::Toggle, {{"instance", "mute"}, {"value", false}},
      {{"instance", "mute"}})
  };
  PropertyObject humidity{};
  humidity.type = PropertyType::Float;
  humidity.retrievable = true;
  humidity.state = {{"instance", "humidity"}, {"value", 40}};
  humidity.parameters = {{"instance", "humidity"}, {"unit", "unit.percent"}};
  humidity.last_updated = 100;
  device.properties = {humidity};
  return device;
}

// Exercise the production controller and the actual QML-facing models. Invoke
// the timer slot explicitly: real timers are stopped after every delivered read,
// and the injected clock advances without sleeps or access to private fields.
struct DeviceFixture {
  double now = 100;
  ScriptedHomeApi api;
  DeviceController controller{&api, nullptr, [this] { return now; }};
  CapabilitiesModel capabilities{&controller};
  PropertiesModel properties{&controller};
  DeviceDataModel data{&controller};
  QList<DeviceController::CapabilitiesList> updates;
  QList<DeviceInfo> raw_snapshots;

  explicit DeviceFixture(DeviceInfo device = InitialDevice()) {
    api.remote = std::move(device);
    QObject::connect(&controller, &DeviceController::capabilitiesUpdateReady,
      &controller, [this](const auto& update) { updates.append(update); });
    QObject::connect(&controller, &DeviceController::deviceDataReady,
      &controller, [this](const auto& snapshot) { raw_snapshots.append(snapshot); });
  }

  bool Load() {
    controller.LoadDevice(api.remote.id);
    return api.device_requests.size() == 1 && ReplyDevice(0, now) &&
      capabilities.rowCount() == api.remote.capabilities.size() && data.IsDeviceOnline();
  }

  int Poll(double time) {
    now = time;
    const int request = api.device_requests.size();
    const bool invoked = QMetaObject::invokeMethod(&controller, "OnTimerTimeout", Qt::DirectConnection);
    controller.StopPolling();
    return invoked && api.device_requests.size() == request + 1 ? request : -1;
  }

  bool ReplyDevice(int request, double time) {
    now = time;
    if (request < 0 || request >= api.device_requests.size()) {
      return false;
    }
    const auto snapshot = api.device_requests[request].snapshot;
    const bool delivered = api.device_requests[request].Reply(snapshot);
    controller.StopPolling();
    return delivered;
  }

  bool PollAndReply(double time) { return ReplyDevice(Poll(time), time); }

  bool DesktopState(int row, const QVariantMap& state, double time) {
    now = time;
    const int previous_count = api.action_requests.size();
    capabilities.UseCapability(row, state);
    return api.action_requests.size() == previous_count + 1;
  }

  bool DesktopValue(int row, const QVariant& value, double time) {
    auto state = capabilities.GetState(row);
    state["value"] = value;
    return DesktopState(row, state, time);
  }

  void RemoteState(int row, const QVariantMap& state, double time) {
    now = time;
    api.remote.capabilities[row].state = state;
    api.remote.capabilities[row].last_updated = time;
  }

  void RemoteValue(int row, const QVariant& value, double time) {
    auto state = api.remote.capabilities[row].state;
    state["value"] = value;
    RemoteState(row, state, time);
  }

  bool ApplyDesktopOnServer(int request, int row, double time) {
    if (request < 0 || request >= api.action_requests.size()) {
      return false;
    }
    const auto& devices = api.action_requests[request].actions;
    if (devices.size() != 1 || devices[0].id != api.remote.id || devices[0].actions.size() != 1 ||
        devices[0].actions[0].type != api.remote.capabilities[row].type) {
      return false;
    }
    RemoteState(row, devices[0].actions[0].state, time);
    return true;
  }

  bool FinishAction(int request, double time, bool success = true) {
    now = time;
    if (request < 0 || request >= api.action_requests.size()) {
      return false;
    }
    const ApiResult<void> result = success ? ApiResult<void>{} :
      ApiResult<void>{std::unexpected(ApiError{ApiErrorKind::Service, "ACTION_FAILED"})};
    return api.action_requests[request].Reply(result);
  }

  QVariant Value(int row = 0) const {
    return capabilities.data(capabilities.index(row), CapabilitiesModel::StateRole).toMap().value("value");
  }
  QVariant PropertyValue() const {
    return properties.data(properties.index(0), PropertiesModel::StateRole).toMap().value("value");
  }
};

void CapabilityCases(bool mobile_only = false) {
  QTest::addColumn<int>("type");
  QTest::addColumn<QVariantMap>("parameters");
  QTest::addColumn<QVariantMap>("initial");
  QTest::addColumn<QVariantMap>("desktop");
  QTest::addColumn<QVariantMap>("mobile");
  QTest::addColumn<bool>("retrievable");

  QTest::newRow("on-off") << int(CapabilityType::OnOff) << QVariantMap{}
    << QVariantMap{{"instance", "on"}, {"value", mobile_only}}
    << QVariantMap{{"instance", "on"}, {"value", true}}
    << QVariantMap{{"instance", "on"}, {"value", false}} << true;
  QTest::newRow("range") << int(CapabilityType::Range) << QVariantMap{{"instance", "brightness"}}
    << QVariantMap{{"instance", "brightness"}, {"value", 20}}
    << QVariantMap{{"instance", "brightness"}, {"value", 80}}
    << QVariantMap{{"instance", "brightness"}, {"value", 35}} << true;
  QTest::newRow("mode") << int(CapabilityType::Mode) << QVariantMap{{"instance", "fan_speed"}}
    << QVariantMap{{"instance", "fan_speed"}, {"value", "low"}}
    << QVariantMap{{"instance", "fan_speed"}, {"value", "high"}}
    << QVariantMap{{"instance", "fan_speed"}, {"value", "medium"}} << true;
  QTest::newRow("toggle") << int(CapabilityType::Toggle) << QVariantMap{{"instance", "mute"}}
    << QVariantMap{{"instance", "mute"}, {"value", mobile_only}}
    << QVariantMap{{"instance", "mute"}, {"value", true}}
    << QVariantMap{{"instance", "mute"}, {"value", false}} << true;
  QTest::newRow("color-rgb") << int(CapabilityType::ColorSetting) << QVariantMap{{"color_model", "rgb"}}
    << QVariantMap{{"instance", "rgb"}, {"value", 0x010203}}
    << QVariantMap{{"instance", "rgb"}, {"value", 0x112233}}
    << QVariantMap{{"instance", "rgb"}, {"value", 0x778899}} << true;
  QTest::newRow("color-temperature") << int(CapabilityType::ColorSetting)
    << QVariantMap{{"color_model", "rgb"}, {"temperature_k", QVariantMap{{"min", 2000}, {"max", 6500}}}}
    << QVariantMap{{"instance", "rgb"}, {"value", 0x010203}}
    << QVariantMap{{"instance", "temperature_k"}, {"value", 3000}}
    << QVariantMap{{"instance", "temperature_k"}, {"value", 5000}} << true;
  QTest::newRow("color-hsv") << int(CapabilityType::ColorSetting) << QVariantMap{{"color_model", "hsv"}}
    << QVariantMap{{"instance", "hsv"}, {"value", QVariantMap{{"h", 0}, {"s", 50}, {"v", 50}}}}
    << QVariantMap{{"instance", "hsv"}, {"value", QVariantMap{{"h", 120}, {"s", 80}, {"v", 80}}}}
    << QVariantMap{{"instance", "hsv"}, {"value", QVariantMap{{"h", 240}, {"s", 90}, {"v", 90}}}} << true;
  QTest::newRow("mobile-switches-color-instance") << int(CapabilityType::ColorSetting)
    << QVariantMap{{"color_model", "rgb"}, {"temperature_k", QVariantMap{{"min", 2000}, {"max", 6500}}}}
    << QVariantMap{{"instance", "rgb"}, {"value", 0x010203}}
    << QVariantMap{{"instance", "rgb"}, {"value", 0x112233}}
    << QVariantMap{{"instance", "temperature_k"}, {"value", 5000}} << true;
  QTest::newRow("color-scene") << int(CapabilityType::ColorSetting) << QVariantMap{{"color_model", "rgb"}}
    << QVariantMap{{"instance", "rgb"}, {"value", 0x010203}}
    << QVariantMap{{"instance", "scene"}, {"value", "reading"}}
    << QVariantMap{{"instance", "scene"}, {"value", "night"}} << true;
  QTest::newRow("non-retrievable-range") << int(CapabilityType::Range)
    << QVariantMap{{"instance", "brightness"}}
    << QVariantMap{{"instance", "brightness"}, {"value", 20}}
    << QVariantMap{{"instance", "brightness"}, {"value", 80}}
    << QVariantMap{{"instance", "brightness"}, {"value", 35}} << false;
}
}

// These are characterization tests: the expectations deliberately include the
// existing surprising outcomes. See docs/device-control-behavior.md before
// changing them during the service extraction.
class DeviceControlTests final : public QObject {
  Q_OBJECT
private slots:
  void MobileChangesAppearOnNextPoll_data() { CapabilityCases(true); }
  void MobileChangesAppearOnNextPoll() {
    QFETCH(int, type);
    QFETCH(QVariantMap, parameters);
    QFETCH(QVariantMap, initial);
    QFETCH(QVariantMap, mobile);
    QFETCH(bool, retrievable);
    auto device = InitialDevice();
    auto capability = Capability(static_cast<CapabilityType::Type>(type), initial, parameters);
    capability.retrievable = retrievable;
    device.capabilities = {capability};
    DeviceFixture fixture(device);
    QVERIFY(fixture.Load());
    QSignalSpy changes(&fixture.capabilities, &QAbstractItemModel::dataChanged);

    fixture.RemoteState(0, mobile, 110);
    QCOMPARE(fixture.capabilities.GetState(0), initial);
    QVERIFY(fixture.PollAndReply(111));
    QCOMPARE(fixture.capabilities.GetState(0), mobile);
    QCOMPARE(changes.size(), 1);
    QVERIFY(fixture.updates.last()[0].has_value());
  }

  void DesktopValueIsOptimisticAndRequestIsScoped() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QSignalSpy changes(&fixture.capabilities, &QAbstractItemModel::dataChanged);
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    QCOMPARE(fixture.Value().toInt(), 80);
    QCOMPARE(changes.size(), 1);
    QCOMPARE(fixture.api.remote.capabilities[0].state.value("value").toInt(), 20);
    QCOMPARE(fixture.api.device_requests.size(), 1); // No immediate readback.

    const auto& devices = fixture.api.action_requests[0].actions;
    QCOMPARE(devices.size(), 1);
    QCOMPARE(devices[0].id, QString("lamp-1"));
    QCOMPARE(devices[0].actions.size(), 1);
    QCOMPARE(devices[0].actions[0].type, CapabilityType::Range);
    QCOMPARE(devices[0].actions[0].state, fixture.capabilities.GetState(0));
    QCOMPARE(fixture.Value(1).toBool(), false);
    // The role is advertised, but the current model does not supply busy state.
    QVERIFY(!fixture.capabilities.data(fixture.capabilities.index(0), CapabilitiesModel::BusyRole).isValid());
  }

  void PendingDesktopActionSuppressesOnlyItsCapability() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    fixture.RemoteValue(0, 35, 111);
    fixture.RemoteValue(1, true, 111);
    fixture.api.remote.capabilities[0].parameters["range"] =
      QVariantMap{{"min", 0}, {"max", 100}, {"precision", 2}};
    fixture.api.remote.properties[0].state["value"] = 55;
    fixture.api.remote.name = "Renamed on mobile";

    // Pending status protects the row even long after the 800 ms grace period.
    QVERIFY(fixture.PollAndReply(150));
    QCOMPARE(fixture.Value().toInt(), 80);
    QCOMPARE(fixture.Value(1).toBool(), true);
    QCOMPARE(fixture.PropertyValue().toInt(), 55);
    QCOMPARE(fixture.data.GetDeviceName(), QString("Renamed on mobile"));
    QCOMPARE(fixture.capabilities.GetParameters(0).value("range").toMap().value("precision").toInt(), 1);
    QVERIFY(!fixture.updates.last()[0].has_value());
    QVERIFY(fixture.updates.last()[1].has_value());
    // Raw device info is emitted before capability suppression is applied.
    QCOMPARE(fixture.raw_snapshots.last().capabilities[0].state.value("value").toInt(), 35);
  }

  void DesktopChangeWinsWhenAppliedAfterMobile() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    fixture.RemoteValue(0, 35, 110.1);
    QVERIFY(fixture.ApplyDesktopOnServer(0, 0, 110.2));
    QVERIFY(fixture.FinishAction(0, 111));
    QVERIFY(fixture.PollAndReply(112));
    QCOMPARE(fixture.Value().toInt(), 80);
    QVERIFY(fixture.updates.last()[0].has_value());
  }

  void MobileChangeWinsAfterDesktopSuppression_data() { CapabilityCases(); }
  void MobileChangeWinsAfterDesktopSuppression() {
    QFETCH(int, type);
    QFETCH(QVariantMap, parameters);
    QFETCH(QVariantMap, initial);
    QFETCH(QVariantMap, desktop);
    QFETCH(QVariantMap, mobile);
    QFETCH(bool, retrievable);
    auto device = InitialDevice();
    auto capability = Capability(static_cast<CapabilityType::Type>(type), initial, parameters);
    capability.retrievable = retrievable;
    device.capabilities = {capability};
    DeviceFixture fixture(device);
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopState(0, desktop, 110));
    QVERIFY(fixture.ApplyDesktopOnServer(0, 0, 110.1));
    fixture.RemoteState(0, mobile, 110.2);
    QVERIFY(fixture.PollAndReply(110.3));
    QCOMPARE(fixture.capabilities.GetState(0), desktop);
    QVERIFY(!fixture.updates.last()[0].has_value());

    // A late acknowledgement does not carry a value or identify the last writer.
    QVERIFY(fixture.FinishAction(0, 111));
    QCOMPARE(fixture.capabilities.GetState(0), desktop);
    QVERIFY(fixture.PollAndReply(111.5));
    QCOMPARE(fixture.capabilities.GetState(0), desktop);
    QVERIFY(!fixture.updates.last()[0].has_value());
    QVERIFY(fixture.PollAndReply(112));
    QCOMPARE(fixture.capabilities.GetState(0), mobile);
    QVERIFY(fixture.updates.last()[0].has_value());
  }

  void SuppressedMobileSnapshotIsNotReplayed() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    QVERIFY(fixture.FinishAction(0, 111));
    fixture.RemoteValue(0, 35, 111.1);
    QVERIFY(fixture.PollAndReply(111.2));
    QCOMPARE(fixture.Value().toInt(), 80);
    QSignalSpy changes(&fixture.capabilities, &QAbstractItemModel::dataChanged);

    fixture.RemoteValue(0, 60, 112);
    QCOMPARE(fixture.Value().toInt(), 80); // Expiry alone does not apply the skipped 35.
    QCOMPARE(changes.size(), 0);
    QVERIFY(fixture.PollAndReply(113));
    QCOMPARE(fixture.Value().toInt(), 60);
  }

  void SuppressionWindowUsesRequestAndReceiveTimes_data() {
    QTest::addColumn<double>("request_time");
    QTest::addColumn<double>("receive_time");
    QTest::addColumn<bool>("accepted");
    // Action begins at 110 and finishes at 111. Lower bound is inclusive,
    // upper bound exclusive; either request or receive time can suppress a row.
    QTest::newRow("request-before-lower-bound") << (110.0 - 0.8 - 0.001) << 112.0 << true;
    QTest::newRow("request-at-lower-bound") << (110.0 - 0.8) << 112.0 << false;
    QTest::newRow("request-after-lower-bound") << (110.0 - 0.8 + 0.001) << 112.0 << false;
    QTest::newRow("request-during-action-delivered-late") << 110.5 << 120.0 << false;
    QTest::newRow("request-before-upper-bound") << (111.0 + 0.8 - 0.001) << 112.0 << false;
    QTest::newRow("request-at-upper-bound") << (111.0 + 0.8) << 112.0 << true;
    QTest::newRow("request-after-upper-bound") << (111.0 + 0.8 + 0.001) << 112.0 << true;
    QTest::newRow("receive-before-upper-bound") << 109.0 << (111.0 + 0.8 - 0.001) << false;
    QTest::newRow("receive-at-upper-bound") << 109.0 << (111.0 + 0.8) << true;
  }

  void SuppressionWindowUsesRequestAndReceiveTimes() {
    QFETCH(double, request_time);
    QFETCH(double, receive_time);
    QFETCH(bool, accepted);
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    fixture.RemoteValue(0, 35, 108);
    int request = -1;
    if (request_time < 110) {
      request = fixture.Poll(request_time);
    }
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    if (request_time >= 110 && request_time < 111) {
      request = fixture.Poll(request_time);
    }
    QVERIFY(fixture.FinishAction(0, 111));
    if (request_time >= 111) {
      request = fixture.Poll(request_time);
    }
    QVERIFY(fixture.ReplyDevice(request, receive_time));
    QCOMPARE(fixture.Value().toInt(), accepted ? 35 : 80);
    QCOMPARE(fixture.updates.last()[0].has_value(), accepted);
  }

  void FailedDesktopActionKeepsOptimisticValueUntilFreshPoll() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QSignalSpy errors(&fixture.controller, &DeviceController::errorOccurred);
    QSignalSpy online_changes(&fixture.data, &DeviceDataModel::deviceStateChanged);
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    fixture.RemoteValue(0, 35, 110.5);
    QVERIFY(fixture.FinishAction(0, 111, false));
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0][0].toString(), QString("ACTION_FAILED"));
    QCOMPARE(fixture.Value().toInt(), 80); // Failure does not roll back the optimistic value.
    QVERIFY(!fixture.data.IsDeviceOnline());
    QCOMPARE(online_changes.size(), 0); // Current failure handler does not notify QML.

    QVERIFY(fixture.PollAndReply(111.5));
    QCOMPARE(fixture.Value().toInt(), 80);
    QVERIFY(fixture.data.IsDeviceOnline());
    QVERIFY(fixture.PollAndReply(112));
    QCOMPARE(fixture.Value().toInt(), 35);
    QCOMPARE(errors.size(), 1);
  }

  void PollFailureDoesNotEndPendingAction() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    const int request = fixture.Poll(111);
    QVERIFY(request >= 0);
    QSignalSpy errors(&fixture.controller, &DeviceController::errorOccurred);
    QVERIFY(fixture.api.device_requests[request].Reply(
      std::unexpected(ApiError{ApiErrorKind::Timeout, "POLL_TIMEOUT"})));
    QCOMPARE(errors.size(), 1);
    QCOMPARE(fixture.Value().toInt(), 80);

    fixture.RemoteValue(0, 35, 112);
    QVERIFY(fixture.PollAndReply(120));
    QCOMPARE(fixture.Value().toInt(), 80);
    QVERIFY(!fixture.updates.last()[0].has_value());
    QVERIFY(fixture.FinishAction(0, 121));
    QVERIFY(fixture.PollAndReply(122));
    QCOMPARE(fixture.Value().toInt(), 35);
  }

  void ConcurrentDesktopActionsShareOnePendingFlag_data() {
    QTest::addColumn<int>("first_reply");
    QTest::addColumn<bool>("success");
    QTest::newRow("older-action-succeeds-first") << 0 << true;
    QTest::newRow("newer-action-succeeds-first") << 1 << true;
    QTest::newRow("older-action-fails-first") << 0 << false;
    QTest::newRow("newer-action-fails-first") << 1 << false;
  }

  void ConcurrentDesktopActionsShareOnePendingFlag() {
    QFETCH(int, first_reply);
    QFETCH(bool, success);
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QSignalSpy errors(&fixture.controller, &DeviceController::errorOccurred);
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    QVERIFY(fixture.DesktopValue(0, 90, 110.2));
    QCOMPARE(fixture.Value().toInt(), 90);
    QCOMPARE(fixture.api.action_requests.size(), 2);
    QVERIFY(fixture.FinishAction(first_reply, 111, success));
    QVERIFY(bool(fixture.api.action_requests[1 - first_reply].handler));

    // Either completion clears the shared flag, even while the other action
    // remains outstanding. A mobile state can then overwrite the latest value.
    fixture.RemoteValue(0, 35, 111.1);
    QVERIFY(fixture.PollAndReply(112));
    QCOMPARE(fixture.Value().toInt(), 35);
    QVERIFY(fixture.updates.last()[0].has_value());
    QCOMPARE(errors.size(), success ? 0 : 1);
    QVERIFY(fixture.FinishAction(1 - first_reply, 113));
    QCOMPARE(fixture.Value().toInt(), 35);
    fixture.RemoteValue(0, 60, 113.1);
    QVERIFY(fixture.PollAndReply(114));
    QCOMPARE(fixture.Value().toInt(), 60);
  }

  void ConcurrentActionsOnDifferentCapabilitiesResumeIndependently() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    QVERIFY(fixture.DesktopValue(1, true, 110.2));
    fixture.RemoteValue(0, 35, 110.3);
    fixture.RemoteValue(1, false, 110.4);
    QVERIFY(fixture.FinishAction(0, 111));
    QVERIFY(fixture.PollAndReply(112));
    QCOMPARE(fixture.Value().toInt(), 35);
    QCOMPARE(fixture.Value(1).toBool(), true);
    QVERIFY(fixture.updates.last()[0].has_value());
    QVERIFY(!fixture.updates.last()[1].has_value());
    QVERIFY(fixture.FinishAction(1, 113));
    QVERIFY(fixture.PollAndReply(114));
    QCOMPARE(fixture.Value(1).toBool(), false);
  }

  void SuppressionIsPerRowForCapabilitiesOfTheSameType() {
    auto device = InitialDevice();
    device.capabilities[1] = Capability(CapabilityType::Range,
      {{"instance", "volume"}, {"value", 30}}, {{"instance", "volume"}});
    DeviceFixture fixture(device);
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    fixture.RemoteValue(0, 35, 111);
    fixture.RemoteValue(1, 60, 111);
    QVERIFY(fixture.PollAndReply(112));
    QCOMPARE(fixture.Value().toInt(), 80);
    QCOMPARE(fixture.Value(1).toInt(), 60);
    QCOMPARE(fixture.capabilities.GetState(1).value("instance").toString(), QString("volume"));
  }

  void OutOfOrderPollRepliesCanRestoreOlderMobileState() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    fixture.RemoteValue(0, 35, 110);
    fixture.api.remote.properties[0].state["value"] = 45;
    const int older = fixture.Poll(110);
    fixture.RemoteValue(0, 60, 111);
    fixture.api.remote.properties[0].state["value"] = 55;
    const int newer = fixture.Poll(111);
    QVERIFY(fixture.ReplyDevice(newer, 112));
    QCOMPARE(fixture.Value().toInt(), 60);
    QCOMPARE(fixture.PropertyValue().toInt(), 55);

    QVERIFY(fixture.ReplyDevice(older, 113));
    QCOMPARE(fixture.Value().toInt(), 35); // Neither request order nor last_updated is checked.
    QCOMPARE(fixture.PropertyValue().toInt(), 45);
    QVERIFY(fixture.PollAndReply(114));
    QCOMPARE(fixture.Value().toInt(), 60);
    QCOMPARE(fixture.PropertyValue().toInt(), 55);
  }

  void NewPollChangesSuppressionOfAnOlderReply_data() {
    QTest::addColumn<bool>("start_new_poll");
    QTest::newRow("only-old-poll-outstanding") << false;
    QTest::newRow("new-poll-overwrites-shared-start-time") << true;
  }

  void NewPollChangesSuppressionOfAnOlderReply() {
    QFETCH(bool, start_new_poll);
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    QVERIFY(fixture.DesktopValue(0, 80, 110));
    const int older = fixture.Poll(110.5); // Captures the pre-action server value, 20.
    QVERIFY(fixture.FinishAction(0, 111));
    fixture.RemoteValue(0, 35, 112);
    const int newer = start_new_poll ? fixture.Poll(112) : -1;

    QVERIFY(fixture.ReplyDevice(older, 113));
    // The decision uses the most recently STARTED poll, not this reply's poll.
    QCOMPARE(fixture.Value().toInt(), start_new_poll ? 20 : 80);
    QCOMPARE(fixture.updates.last()[0].has_value(), start_new_poll);
    if (start_new_poll) {
      QVERIFY(fixture.ReplyDevice(newer, 114));
    } else {
      QVERIFY(fixture.PollAndReply(114));
    }
    QCOMPARE(fixture.Value().toInt(), 35);
  }

  void PreviousDeviceMobileSnapshotIsIgnoredAfterSwitch() {
    DeviceFixture fixture;
    QVERIFY(fixture.Load());
    fixture.RemoteValue(0, 35, 110);
    const int older = fixture.Poll(110);
    fixture.api.remote = InitialDevice();
    fixture.api.remote.id = "lamp-2";
    fixture.api.remote.capabilities[0].state["value"] = 60;
    fixture.now = 111;
    fixture.controller.LoadDevice("lamp-2");
    QCOMPARE(fixture.api.device_requests.size(), 3);
    QVERIFY(fixture.ReplyDevice(2, 111));
    QCOMPARE(fixture.Value().toInt(), 60);
    const int update_count = fixture.updates.size();
    QVERIFY(fixture.ReplyDevice(older, 112));
    QCOMPARE(fixture.Value().toInt(), 60);
    QCOMPARE(fixture.updates.size(), update_count);
  }
};

QTEST_GUILESS_MAIN(DeviceControlTests)
#include "DeviceControlTests.moc"
