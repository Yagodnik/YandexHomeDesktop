#include <QPointer>
#include <QTest>
#include <memory>

#include "services/DeviceSession.h"

namespace {
template<typename T>
struct Reply {
  QPointer<QObject> context;
  ApiResultHandler<T> handler;
  bool Send(ApiResult<T> result) {
    if (!context || !handler) {
      return false;
    }
    auto callback = std::move(handler);
    callback(std::move(result));
    return true;
  }
};

class FakeHomeApi final : public IHomeApi {
public:
  struct Read : Reply<DeviceInfo> { QString device_id; };
  struct Command : Reply<void> { QList<DeviceActionsObject> actions; };
  QList<Read> reads;
  QList<Command> commands;
  std::optional<DeviceInfo> immediate;

  void GetDeviceInfo(const QString& id, QObject* context, ApiResultHandler<DeviceInfo> handler) override {
    if (immediate) {
      handler(*immediate);
    } else {
      reads.append({{context, std::move(handler)}, id});
    }
  }
  void PerformActions(const QList<DeviceActionsObject>& actions, QObject* context,
                      ApiResultHandler<void> handler) override {
    commands.append({{context, std::move(handler)}, actions});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo>) override { QFAIL("Unexpected user request"); }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>>) override { QFAIL("Unexpected scenarios request"); }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void>) override { QFAIL("Unexpected scenario action"); }
};

DeviceInfo Device(const QString& id, int brightness) {
  DeviceInfo info{};
  info.id = id;
  info.status = Status::Ok;
  info.state = DeviceState::Online;
  CapabilityObject capability{};
  capability.type = CapabilityType::Range;
  capability.state = {{"instance", "brightness"}, {"value", brightness}};
  info.capabilities = {capability};
  return info;
}
}

// This executable links AppServices and Qt Core/Test only. No Models, Qt Gui,
// QML, auth, platform integration, or real transport is needed for device control.
class DeviceServiceTests final : public QObject {
  Q_OBJECT
private slots:
  void ValidatedCommandsRejectBeforeDispatchAndCheckFreshMetadata() {
    FakeHomeApi api;
    DeviceService service(&api);
    QObject caller;
    std::optional<CommandResult> result;
    const auto complete = [&result](CommandResult value) { result = std::move(value); };

    service.ApplyCapability("lamp", CapabilityType::Range,
                            {{"instance", "brightness"}, {"value", "bad"}}, &caller, complete);
    QVERIFY(result && !*result);
    QCOMPARE(std::get<CommandRejection>(result->error()).reason, CommandRejectionReason::InvalidValue);
    QVERIFY(api.reads.isEmpty());

    auto device = Device("lamp", 20);
    device.capabilities[0].parameters = {
      {"instance", "brightness"}, {"range", QVariantMap{{"min", 0}, {"max", 100}}}
    };
    const QList<QPair<DeviceInfo, CommandRejectionReason>> cases{
      {Device("other", 20), CommandRejectionReason::InvalidDeviceResponse},
      {DeviceInfo{.status = Status::Ok, .id = "lamp"}, CommandRejectionReason::CapabilityNotFound},
      {[&device] {
        auto duplicate = device;
        duplicate.capabilities.append(device.capabilities[0]);
        return duplicate;
      }(), CommandRejectionReason::AmbiguousCapability},
      {device, CommandRejectionReason::InvalidValue}
    };
    for (const auto& [snapshot, reason] : cases) {
      result.reset();
      service.ApplyCapability("lamp", CapabilityType::Range,
                              {{"instance", "brightness"}, {"value", 101}}, &caller, complete);
      QVERIFY(!result);
      QVERIFY(api.reads.last().Send(snapshot));
      QVERIFY(result && !*result);
      QCOMPARE(std::get<CommandRejection>(result->error()).reason, reason);
      QVERIFY(api.commands.isEmpty());
    }
  }

  void ValidatedCommandsPreserveInlineReadsErrorsAndCallerLifetime() {
    FakeHomeApi api;
    api.immediate = Device("lamp", 20);
    DeviceService service(&api);
    auto caller = std::make_unique<QObject>();
    std::optional<CommandResult> result;
    const auto complete = [&result](CommandResult value) { result = std::move(value); };
    const QVariantMap state{{"instance", "brightness"}, {"value", -5}, {"relative", true}};

    service.ApplyCapability("lamp", CapabilityType::Range, state, caller.get(), complete);
    QVERIFY(!result);
    QCOMPARE(api.commands.size(), 1);
    QCOMPARE(api.commands.last().actions[0].actions[0].state, state);
    QVERIFY(api.commands.last().Send(std::unexpected(ApiError{ApiErrorKind::Http, "expired", 401})));
    QVERIFY(result && !*result);
    QCOMPARE(std::get<ApiError>(result->error()).http_status, 401);

    result.reset();
    service.ApplyCapability("lamp", CapabilityType::Range, state, caller.get(), complete);
    caller.reset();
    QVERIFY(!api.commands.last().Send(ApiResult<void>{}));
    QVERIFY(!result);

    api.immediate.reset();
    caller = std::make_unique<QObject>();
    service.ApplyCapability("lamp", CapabilityType::Range, state, caller.get(), complete);
    caller.reset();
    QVERIFY(!api.reads.last().Send(Device("lamp", 20)));
    QCOMPARE(api.commands.size(), 2);

    caller = std::make_unique<QObject>();
    auto transient = std::make_unique<DeviceService>(&api);
    transient->ApplyCapability("lamp", CapabilityType::Range, state, caller.get(), complete);
    transient.reset();
    QVERIFY(api.reads.last().Send(Device("lamp", 20)));
    QVERIFY(!result);
    QCOMPARE(api.commands.size(), 2);
  }

  void CommandsUseExplicitIdsWithoutSelectingOrLoading() {
    FakeHomeApi api;
    DeviceService service(&api);
    QObject cli, rest;
    int successes = 0;
    std::optional<ApiError> error;
    const QVariantMap state{{"instance", "brightness"}, {"value", -10}, {"relative", true}};
    service.UseCapability("lamp-a", CapabilityType::Range, state, &cli,
      [&successes](ApiResult<void> result) { if (result) { ++successes; } });
    service.UseCapability("lamp-b", CapabilityType::OnOff, {{"instance", "on"}, {"value", true}}, &rest,
      [&error](ApiResult<void> result) { if (!result) { error = result.error(); } });

    QVERIFY(api.reads.isEmpty());
    QCOMPARE(api.commands.size(), 2);
    QCOMPARE(api.commands[0].context.data(), &cli);
    QCOMPARE(api.commands[1].context.data(), &rest);
    QCOMPARE(api.commands[0].actions.size(), 1);
    QCOMPARE(api.commands[0].actions[0].id, QString("lamp-a"));
    QCOMPARE(api.commands[1].actions[0].id, QString("lamp-b"));
    QCOMPARE(api.commands[0].actions[0].actions.size(), 1);
    const auto& action = api.commands[0].actions[0].actions[0];
    QCOMPARE(action.type, CapabilityType{CapabilityType::Range});
    QCOMPARE(action.state, state);
    QVERIFY(action.parameters.isEmpty());

    // Results are delivered to their caller even when completions reverse order.
    QVERIFY(api.commands[1].Send(std::unexpected(ApiError{ApiErrorKind::Http, "failed", 503})));
    QVERIFY(error);
    QCOMPARE(error->kind, ApiErrorKind::Http);
    QCOMPARE(error->message, QString("failed"));
    QCOMPARE(error->http_status, 503);
    QCOMPARE(successes, 0);
    QVERIFY(api.commands[0].Send(ApiResult<void>{}));
    QCOMPARE(successes, 1);
  }

  void ReadsPreserveSnapshotsAndStructuredErrors() {
    FakeHomeApi api;
    DeviceService service(&api);
    QObject caller;
    std::optional<DeviceInfo> received;
    std::optional<ApiError> error;
    service.GetDeviceInfo("lamp-a", &caller, [&received](ApiResult<DeviceInfo> result) {
      if (result) { received = std::move(*result); }
    });
    service.GetDeviceInfo("lamp-b", &caller, [&error](ApiResult<DeviceInfo> result) {
      if (!result) { error = result.error(); }
    });
    QCOMPARE(api.reads[0].device_id, QString("lamp-a"));
    QCOMPARE(api.reads[1].device_id, QString("lamp-b"));
    QVERIFY(api.reads[1].Send(std::unexpected(ApiError{ApiErrorKind::Timeout, "timeout"})));
    QVERIFY(api.reads[0].Send(Device("lamp-a", 80)));
    QVERIFY(received);
    QCOMPARE(received->id, QString("lamp-a"));
    QCOMPARE(received->capabilities[0].state.value("value").toInt(), 80);
    QVERIFY(error);
    QCOMPARE(error->kind, ApiErrorKind::Timeout);
    QCOMPARE(error->message, QString("timeout"));
  }

  void CallerDestructionCancelsReadsAndCommands() {
    FakeHomeApi api;
    DeviceService service(&api);
    auto caller = std::make_unique<QObject>();
    int deliveries = 0;
    service.GetDeviceInfo("lamp-a", caller.get(), [&deliveries](ApiResult<DeviceInfo>) { ++deliveries; });
    service.UseCapability("lamp-a", CapabilityType::OnOff, {}, caller.get(),
      [&deliveries](ApiResult<void>) { ++deliveries; });
    caller.reset();
    QVERIFY(!api.reads[0].Send(Device("lamp-a", 20)));
    QVERIFY(!api.commands[0].Send(ApiResult<void>{}));
    QCOMPARE(deliveries, 0);
  }

  void SessionsShareServiceButKeepSelectionAndSuppressionSeparate() {
    FakeHomeApi api;
    DeviceService service(&api);
    double now = 100;
    DeviceSession desktop(&service, nullptr, [&now] { return now; });
    DeviceSession observer(&service, nullptr, [&now] { return now; });
    DeviceSession::CapabilitiesList desktop_update, observer_update;
    connect(&desktop, &DeviceSession::capabilitiesUpdateReady, this,
      [&desktop_update](const auto& update) { desktop_update = update; });
    connect(&observer, &DeviceSession::capabilitiesUpdateReady, this,
      [&observer_update](const auto& update) { observer_update = update; });
    desktop.LoadDevice("lamp-a");
    observer.LoadDevice("lamp-b");
    QVERIFY(api.reads[0].Send(Device("lamp-a", 20)));
    QVERIFY(api.reads[1].Send(Device("lamp-b", 30)));
    desktop.StopPolling();
    observer.StopPolling();

    now = 110;
    desktop.UseCapability(0, CapabilityType::Range, {{"instance", "brightness"}, {"value", 80}});
    now = 120;
    desktop.Refresh();
    observer.Refresh();
    QCOMPARE(api.reads[2].device_id, QString("lamp-a"));
    QCOMPARE(api.reads[3].device_id, QString("lamp-b"));
    QVERIFY(api.reads[3].Send(Device("lamp-b", 60)));
    QVERIFY(api.reads[2].Send(Device("lamp-a", 35)));
    desktop.StopPolling();
    observer.StopPolling();
    QVERIFY(!desktop_update[0]);
    QVERIFY(observer_update[0]);
    QCOMPARE(observer_update[0]->state.value("value").toInt(), 60);

    observer.ForgetDevice();
    now = 121;
    QVERIFY(api.commands[0].Send(ApiResult<void>{}));
    now = 122;
    desktop.Refresh();
    QVERIFY(api.reads[4].Send(Device("lamp-a", 35)));
    desktop.StopPolling();
    QVERIFY(desktop_update[0]);
    QCOMPARE(desktop_update[0]->state.value("value").toInt(), 35);
  }

  void SessionDestructionCancelsItsOutstandingOperations() {
    FakeHomeApi api;
    DeviceService service(&api);
    auto session = std::make_unique<DeviceSession>(&service);
    session->LoadDevice("lamp-a");
    QVERIFY(api.reads[0].Send(Device("lamp-a", 20)));
    session->UseCapability(0, CapabilityType::OnOff, {{"instance", "on"}, {"value", true}});
    session->Refresh();
    session.reset();
    QVERIFY(!api.commands[0].Send(ApiResult<void>{}));
    QVERIFY(!api.reads[1].Send(Device("lamp-a", 35)));

    QObject caller;
    bool delivered = false;
    service.GetDeviceInfo("lamp-b", &caller, [&delivered](ApiResult<DeviceInfo> result) { delivered = bool(result); });
    QVERIFY(api.reads[2].Send(Device("lamp-b", 30)));
    QVERIFY(delivered);
  }

  void SessionResetRejectsOldReadsAndCommandsForTheSameDevice() {
    FakeHomeApi api;
    DeviceService service(&api);
    DeviceSession session(&service, nullptr, [] { return 100.; });
    int errors = 0;
    connect(&session, &DeviceSession::errorOccurred, this, [&errors] { ++errors; });
    session.LoadDevice("lamp");
    QVERIFY(api.reads[0].Send(Device("lamp", 20)));
    session.UseCapability(0, CapabilityType::Range, {{"instance", "brightness"}, {"value", 80}});
    session.Refresh();
    session.ResetSession();
    QVERIFY(!session.IsPolling());
    session.ContinuePollingIfNeeded();
    QVERIFY(!session.IsPolling());
    session.LoadDevice("lamp");
    QVERIFY(api.reads[1].Send(Device("lamp", 35)));
    QVERIFY(api.commands[0].Send(std::unexpected(ApiError{ApiErrorKind::Service, "old error"})));
    QCOMPARE(errors, 0);
    QVERIFY(!session.IsPolling());
    QVERIFY(api.reads[2].Send(Device("lamp", 50)));
    QVERIFY(session.IsPolling());
    session.StopPolling();
  }

  void SynchronousResponsePreservesResetAndUpdateOrder() {
    FakeHomeApi api;
    api.immediate = Device("lamp-a", 20);
    DeviceService service(&api);
    DeviceSession session(&service);
    QStringList events;
    connect(&session, &DeviceSession::loadRequestMade, this, [&events] { events << "reset"; });
    connect(&session, &DeviceSession::deviceInfoReceived, this, [&events] { events << "received"; });
    connect(&session, &DeviceSession::deviceDataReady, this, [&events] { events << "metadata"; });
    connect(&session, &DeviceSession::capabilitiesUpdateReady, this, [&events] { events << "capabilities"; });
    connect(&session, &DeviceSession::propertiesUpdateReady, this, [&events] { events << "properties"; });
    session.LoadDevice("lamp-a");
    session.StopPolling();
    QCOMPARE(events, QStringList({"reset", "received", "metadata", "capabilities", "properties"}));
  }

  void PollingLifecyclePreservesLateReplyRestart() {
    FakeHomeApi api;
    DeviceService service(&api);
    DeviceSession session(&service);
    session.LoadDevice("lamp-a");
    QVERIFY(!session.IsPolling());
    QVERIFY(api.reads[0].Send(Device("lamp-a", 20)));
    QVERIFY(session.IsPolling());
    session.StopPolling();
    QVERIFY(!session.IsPolling());
    session.ContinuePollingIfNeeded();
    QVERIFY(session.IsPolling());
    session.Refresh();
    session.StopPolling();
    QVERIFY(api.reads[1].Send(Device("lamp-a", 35)));
    // Compatibility: stopping is not a persistent pause; a response restarts it.
    QVERIFY(session.IsPolling());
    session.Refresh();
    session.ForgetDevice();
    session.ContinuePollingIfNeeded();
    QVERIFY(!session.IsPolling());
    QVERIFY(api.reads[2].Send(Device("lamp-a", 60)));
    QVERIFY(!session.IsPolling());
  }
};

QTEST_GUILESS_MAIN(DeviceServiceTests)
#include "DeviceServiceTests.moc"
