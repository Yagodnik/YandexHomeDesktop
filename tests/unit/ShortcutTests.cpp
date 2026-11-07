#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include "services/ShortcutRuntime.h"
#include "services/ShortcutControlService.h"
#include "serialization/Serialization.h"
#include "worker/WorkerControlServer.h"

namespace {
template <typename T> struct Reply {
  QPointer<QObject> context;
  ApiResultHandler<T> handler;
  bool Send(ApiResult<T> value) {
    if (!context || !handler)
      return false;
    auto callback = std::move(handler);
    callback(std::move(value));
    return true;
  }
};
class Api final : public IHomeApi {
public:
  QList<Reply<DeviceInfo>> reads;
  QList<Reply<void>> writes;
  QList<DeviceActionsObject> sent;
  QList<Reply<QList<ScenarioObject>>> scenario_reads;
  struct ScenarioWrite : Reply<void> {
    QString id;
  };
  QList<ScenarioWrite> scenario_writes;
  std::optional<QList<ScenarioObject>> immediate_scenarios;
  std::optional<DeviceInfo> immediate;
  bool inline_write = false;
  void GetDeviceInfo(const QString&, QObject* context,
                     ApiResultHandler<DeviceInfo> handler) override {
    if (immediate)
      handler(*immediate);
    else
      reads.append({context, std::move(handler)});
  }
  void PerformActions(const QList<DeviceActionsObject>& actions, QObject* context,
                      ApiResultHandler<void> handler) override {
    sent.append(actions);
    if (inline_write)
      handler({});
    else
      writes.append({context, std::move(handler)});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo>) override {
    QFAIL("Unexpected request");
  }
  void GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) override {
    if (immediate_scenarios)
      handler(*immediate_scenarios);
    else
      scenario_reads.append({context, std::move(handler)});
  }
  void ExecuteScenario(const QString& id, QObject* context,
                       ApiResultHandler<void> handler) override {
    if (inline_write)
      handler({});
    else
      scenario_writes.append({{context, std::move(handler)}, id});
  }
};
class Registrar final : public IShortcutRegistrar {
public:
  QHash<int, std::function<void()>> callbacks;
  QSet<int> conflicts;
  bool refuse_clear = false;
  int registrations = 0;
  bool Clear() override {
    if (refuse_clear)
      return false;
    callbacks.clear();
    return true;
  }
  bool Register(int combination, std::function<void()> callback) override {
    ++registrations;
    if (conflicts.contains(combination))
      return false;
    callbacks.insert(combination, std::move(callback));
    return true;
  }
};
KeybindTarget Target(QString kind = "on_off", QString instance = "on", QString action = "toggle",
                     QString value = {}) {
  return {.device_id = "lamp",
          .capability_type = "devices.capabilities." + kind,
          .instance = instance,
          .action = action,
          .value = value};
}
KeybindAssignment
Binding(int key = QKeyCombination(Qt::ControlModifier | Qt::AltModifier, Qt::Key_K).toCombined(),
        KeybindTarget target = Target()) {
  return {.target = target, .combination = key, .device_name = "Lamp", .action_label = "Switch"};
}
DeviceInfo Device(CapabilityType type = CapabilityType::OnOff,
                  QVariantMap state = {{"instance", "on"}, {"value", false}},
                  QVariantMap parameters = {{"instance", "on"}}) {
  DeviceInfo info{};
  info.status = Status::Ok;
  info.state = DeviceState::Online;
  info.id = "lamp";
  info.capabilities = {CapabilityObject{
      .type = type, .retrievable = true, .state = state, .parameters = parameters}};
  return info;
}
QJsonObject Configure(QList<KeybindAssignment> bindings = {Binding()}, bool background = false,
                      bool recording = false, QString client = "gui") {
  QJsonArray encoded;
  for (const auto& binding : bindings)
    encoded.append(Serialization::To(binding));
  return {{"command", "configure"}, {"client", client},         {"account_id", "account"},
          {"bindings", encoded},    {"background", background}, {"recording", recording}};
}
struct Harness {
  Api api;
  DeviceService devices{&api};
  ScenarioService scenarios{&api};
  ShortcutActionService actions{&devices, &scenarios, nullptr};
  Registrar registrar;
  ShortcutRuntime runtime{&registrar, &actions, "account"};
};
} // namespace

class ShortcutTests final : public QObject {
  Q_OBJECT
private slots:
  void ScenarioShortcutsValidateFreshStateAndHaveIndependentGuards() {
    Harness h;
    const KeybindTarget scenario{.action = "run", .scenario_id = "lamp"};
    const auto device_binding = Binding();
    const auto scenario_binding = Binding(Qt::Key_F8, scenario);
    QVERIFY(h.runtime.Handle(Configure({device_binding, scenario_binding}))["ok"].toBool());
    const auto run = h.registrar.callbacks.value(scenario_binding.combination);
    run();
    run();
    QCOMPARE(h.api.scenario_reads.size(), 1);
    h.registrar.callbacks.value(device_binding.combination)();
    QCOMPARE(h.api.reads.size(), 1); // A device and scenario may share the same ID.
    QVERIFY(h.api.scenario_reads[0].Send(
        QList<ScenarioObject>{ScenarioObject{.id = "lamp", .name = "Renamed", .is_active = true}}));
    QCOMPARE(h.api.scenario_writes.size(), 1);
    QCOMPARE(h.api.scenario_writes[0].id, QString("lamp"));
    run();
    QCOMPARE(h.api.scenario_reads.size(), 1);
    QVERIFY(h.api.scenario_writes[0].Send({}));
    QCOMPARE(h.runtime.Status()["completed"].toInt(), 1);
    run();
    QCOMPARE(h.api.scenario_reads.size(), 2);
    QVERIFY(h.api.scenario_reads[1].Send(
        QList<ScenarioObject>{ScenarioObject{.id = "lamp", .is_active = false}}));
    QCOMPARE(h.api.scenario_writes.size(), 1);
    QCOMPARE(h.runtime.Status()["action_error"].toString(), QString("scenario_inactive"));
  }
  void MissingInactiveAndFailedScenariosNeverExecute_data() {
    QTest::addColumn<QString>("condition");
    for (const auto* condition : {"missing", "inactive", "api_error"})
      QTest::newRow(condition) << QString(condition);
  }
  void MissingInactiveAndFailedScenariosNeverExecute() {
    QFETCH(QString, condition);
    Harness h;
    QSignalSpy result(&h.actions, &ShortcutActionService::finished);
    const KeybindTarget scenario{.action = "run", .scenario_id = "evening"};
    h.actions.Activate(scenario);
    if (condition == "api_error")
      QVERIFY(h.api.scenario_reads[0].Send(
          std::unexpected(ApiError{ApiErrorKind::Http, "synthetic", 401})));
    else
      QVERIFY(h.api.scenario_reads[0].Send(QList<ScenarioObject>{
          ScenarioObject{.id = condition == "missing" ? "other" : "evening", .is_active = false}}));
    QVERIFY(h.api.scenario_writes.isEmpty());
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0][1].toString(), condition == "api_error"  ? QString("api_error")
                                      : condition == "inactive" ? QString("scenario_inactive")
                                                                : QString("scenario_unavailable"));
    h.actions.Activate(scenario);
    QCOMPARE(h.api.scenario_reads.size(), 2);
  }
  void ScenarioReloadPauseAndStopCancelBothExecutionPhases() {
    Harness h;
    const KeybindTarget scenario{.action = "run", .scenario_id = "evening"};
    const auto binding = Binding(Qt::Key_F8, scenario);
    h.runtime.Handle(Configure({binding}));
    h.registrar.callbacks.value(binding.combination)();
    const auto late_read = h.api.scenario_reads[0].handler;
    h.runtime.Handle(Configure({binding}, false, true));
    QVERIFY(!h.api.scenario_reads[0].Send(QList<ScenarioObject>{}));
    late_read(QList<ScenarioObject>{ScenarioObject{.id = "evening", .is_active = true}});
    QVERIFY(h.api.scenario_writes.isEmpty());
    h.runtime.Handle(Configure({binding}));
    h.registrar.callbacks.value(binding.combination)();
    QVERIFY(h.api.scenario_reads[1].Send(
        QList<ScenarioObject>{ScenarioObject{.id = "evening", .is_active = true}}));
    const auto late_write = h.api.scenario_writes[0].handler;
    h.runtime.Handle(Configure({}));
    QVERIFY(!h.api.scenario_writes[0].Send({}));
    late_write({});
    QCOMPARE(h.runtime.Status()["completed"].toInt(), 0);
    h.runtime.Handle(Configure({binding}));
    h.registrar.callbacks.value(binding.combination)();
    h.runtime.Stop();
    QVERIFY(!h.api.scenario_reads[2].Send(QList<ScenarioObject>{}));
  }
  void InlineScenarioResultsAndReentrantDeadlineRespectCancellation() {
    Harness h;
    const KeybindTarget scenario{.action = "run", .scenario_id = "evening"};
    h.api.immediate_scenarios =
        QList<ScenarioObject>{ScenarioObject{.id = "evening", .is_active = true}};
    h.api.inline_write = true;
    QSignalSpy results(&h.actions, &ShortcutActionService::finished);
    h.actions.Activate(scenario);
    h.actions.Activate(scenario);
    QCOMPARE(results.size(), 2);
    QCOMPARE(results[1][1].toString(), QString{});
    h.api.immediate_scenarios.reset();
    ShortcutActionService timed(&h.devices, &h.scenarios, nullptr, 10);
    QSignalSpy expired(&timed, &ShortcutActionService::finished);
    timed.Activate(scenario);
    const auto late_read = h.api.scenario_reads.last().handler;
    connect(&timed, &ShortcutActionService::finished, this, [&](const QString&, const QString&) {
      late_read(QList<ScenarioObject>{ScenarioObject{.id = "evening", .is_active = true}});
    });
    QTRY_COMPARE(expired.size(), 1);
    QCOMPARE(expired[0][1].toString(), QString("timeout"));
    QVERIFY(h.api.scenario_writes.isEmpty());
  }
  void WorkerAcceptsLegacyDeviceBindingsAndRejectsDuplicateScenarioTargets() {
    Harness h;
    auto request = Configure();
    auto bindings = request["bindings"].toArray();
    auto binding = bindings[0].toObject();
    auto target = binding["target"].toObject();
    target.remove("scenario_id");
    binding["target"] = target;
    request["bindings"] = QJsonArray{binding};
    QVERIFY(h.runtime.Handle(request)["ok"].toBool());
    const KeybindTarget scenario{.action = "run", .scenario_id = "evening"};
    QVERIFY(!h.runtime
                 .Handle(Configure(
                     {Binding(Qt::Key_F8, scenario), Binding(Qt::Key_F9, scenario)}))["ok"]
                 .toBool());
    QCOMPARE(h.runtime.Status()["registered"].toInt(), 1);
  }
  void SavedBindingDispatchesFreshToggleOncePerDevice() {
    Harness h;
    QVERIFY(h.runtime.Handle(Configure())["ok"].toBool());
    const auto activate = h.registrar.callbacks.value(Binding().combination);
    activate();
    activate();
    QCOMPARE(h.api.reads.size(), 1);
    QVERIFY(h.api.reads[0].Send(Device()));
    QCOMPARE(h.api.sent.size(), 1);
    QCOMPARE(h.api.sent[0].id, QString("lamp"));
    QCOMPARE(h.api.sent[0].actions[0].state.value("value"), QVariant(true));
    activate();
    QCOMPARE(h.api.reads.size(), 1);
    QVERIFY(h.api.writes[0].Send({}));
    activate();
    QCOMPARE(h.api.reads.size(), 2);
    auto on = Device();
    on.capabilities[0].state["value"] = true;
    QVERIFY(h.api.reads[1].Send(on));
    QCOMPARE(h.api.sent[1].actions[0].state.value("value"), QVariant(false));
  }
  void ActionPayloads_data() {
    QTest::addColumn<QString>("kind");
    QTest::addColumn<QString>("instance");
    QTest::addColumn<QString>("action");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QVariantMap>("parameters");
    QTest::addColumn<QVariantMap>("expected");
    QTest::newRow("toggle") << "toggle" << "mute" << "toggle" << ""
                            << QVariantMap{{"instance", "mute"}}
                            << QVariantMap{{"instance", "mute"}, {"value", true}};
    QTest::newRow("increase") << "range" << "brightness" << "increase" << ""
                              << QVariantMap{{"instance", "brightness"},
                                             {"range", QVariantMap{{"precision", 5}}}}
                              << QVariantMap{
                                     {"instance", "brightness"}, {"value", 5.}, {"relative", true}};
    QTest::newRow("decrease-relative-only")
        << "range" << "volume" << "decrease" << ""
        << QVariantMap{{"instance", "volume"}, {"random_access", false}}
        << QVariantMap{{"instance", "volume"}, {"value", -1.}, {"relative", true}};
    QTest::newRow("mode") << "mode" << "fan_speed" << "set_mode" << "auto"
                          << QVariantMap{{"instance", "fan_speed"},
                                         {"modes", QVariantList{QVariantMap{{"value", "auto"}}}}}
                          << QVariantMap{{"instance", "fan_speed"}, {"value", "auto"}};
    QTest::newRow("rgb") << "color_setting" << "" << "set_color" << "#ff0000"
                         << QVariantMap{{"color_model", "rgb"}}
                         << QVariantMap{{"instance", "rgb"}, {"value", 0xff0000u}};
    QTest::newRow("hsv") << "color_setting" << "" << "set_color" << "#00ff00"
                         << QVariantMap{{"color_model", "hsv"}}
                         << QVariantMap{
                                {"instance", "hsv"},
                                {"value", QVariantMap{{"h", 120.}, {"s", 100.}, {"v", 100.}}}};
    QTest::newRow("black-hsv") << "color_setting" << "" << "set_color" << "#000000"
                               << QVariantMap{{"color_model", "hsv"}}
                               << QVariantMap{
                                      {"instance", "hsv"},
                                      {"value", QVariantMap{{"h", 0.}, {"s", 0.}, {"v", 0.}}}};
    QTest::newRow("temperature") << "color_setting" << "" << "set_temperature" << "4000"
                                 << QVariantMap{{"temperature_k",
                                                 QVariantMap{{"min", 2000}, {"max", 6500}}}}
                                 << QVariantMap{{"instance", "temperature_k"}, {"value", 4000}};
    QTest::newRow("scene")
        << "color_setting" << "" << "set_scene" << "sunset"
        << QVariantMap{{"color_scene",
                        QVariantMap{{"scenes", QVariantList{QVariantMap{{"id", "sunset"}}}}}}}
        << QVariantMap{{"instance", "scene"}, {"value", "sunset"}};
  }
  void ActionPayloads() {
    QFETCH(QString, kind);
    QFETCH(QString, instance);
    QFETCH(QString, action);
    QFETCH(QString, value);
    QFETCH(QVariantMap, parameters);
    QFETCH(QVariantMap, expected);
    Harness h;
    const auto target = Target(kind, instance, action, value);
    h.actions.Activate(target);
    QVERIFY(h.api.reads[0].Send(Device(CapabilityType::AsValue(target.capability_type),
                                       {{"instance", instance}, {"value", false}}, parameters)));
    QCOMPARE(h.api.sent.size(), 1);
    QCOMPARE(h.api.sent[0].actions[0].state, expected);
  }
  void InvalidFreshSnapshotsNeverWrite_data() {
    QTest::addColumn<int>("variant");
    for (int variant = 0; variant < 10; ++variant)
      QTest::newRow(qPrintable(QString::number(variant))) << variant;
  }
  void InvalidFreshSnapshotsNeverWrite() {
    QFETCH(int, variant);
    Harness h;
    QSignalSpy result(&h.actions, &ShortcutActionService::finished);
    h.actions.Activate(Target());
    auto snapshot = Device();
    switch (variant) {
    case 0:
      snapshot.id = "other";
      break;
    case 1:
      snapshot.status = Status::Unknown;
      break;
    case 2:
      snapshot.state = DeviceState::Offline;
      break;
    case 3:
      snapshot.capabilities = {};
      break;
    case 4:
      snapshot.capabilities.append(snapshot.capabilities[0]);
      break;
    case 5:
      snapshot.capabilities[0].retrievable = false;
      break;
    case 6:
      snapshot.capabilities[0].state.remove("value");
      break;
    case 7:
      snapshot.capabilities[0].state["value"] = "false";
      break;
    case 8:
      snapshot.capabilities[0].state["instance"] = "other";
      break;
    case 9:
      snapshot.capabilities.append(snapshot.capabilities[0]);
      snapshot.capabilities[1].retrievable = false;
      break;
    }
    QVERIFY(h.api.reads[0].Send(snapshot));
    QVERIFY(h.api.sent.isEmpty());
    QCOMPARE(result.size(), 1);
    QVERIFY(!result[0][1].toString().isEmpty());
  }
  void UnsupportedSavedValueIsValidatedAgainstCurrentDevice() {
    Harness h;
    h.actions.Activate(Target("mode", "fan_speed", "set_mode", "auto"));
    QVERIFY(h.api.reads[0].Send(Device(
        CapabilityType::Mode, {},
        {{"instance", "fan_speed"}, {"modes", QVariantList{QVariantMap{{"value", "low"}}}}})));
    QVERIFY(h.api.sent.isEmpty());
  }
  void ReloadPauseAndStopCancelPendingCallbacks() {
    Harness h;
    h.runtime.Handle(Configure());
    h.registrar.callbacks.value(Binding().combination)();
    h.runtime.Handle(Configure({Binding()}, false, true));
    QVERIFY(h.registrar.callbacks.isEmpty());
    QVERIFY(!h.api.reads[0].Send(Device()));
    QVERIFY(h.api.sent.isEmpty());
    h.runtime.Handle(Configure({Binding()}, false, false));
    h.registrar.callbacks.value(Binding().combination)();
    QVERIFY(h.api.reads[1].Send(Device()));
    h.runtime.Handle(Configure({}));
    QVERIFY(!h.api.writes[0].Send({}));
    QVERIFY(h.registrar.callbacks.isEmpty());
    h.runtime.Handle(Configure());
    h.registrar.callbacks.value(Binding().combination)();
    h.runtime.Stop();
    QVERIFY(!h.api.reads[2].Send(Device()));
    QVERIFY(h.registrar.callbacks.isEmpty());
  }
  void IdempotentHeartbeatKeepsNativeRegistrationAndPendingCommand() {
    Harness h;
    h.runtime.Handle(Configure());
    h.registrar.callbacks.value(Binding().combination)();
    h.runtime.Handle(Configure());
    QCOMPARE(h.registrar.registrations, 1);
    QVERIFY(h.api.reads[0].Send(Device()));
    QCOMPARE(h.api.sent.size(), 1);
  }
  void RegistrationConflictsAreReportedWithoutDisablingOtherBindings() {
    Harness h;
    auto other = Binding(QKeyCombination(Qt::ControlModifier, Qt::Key_J).toCombined(),
                         Target("range", "brightness", "increase"));
    h.registrar.conflicts.insert(Binding().combination);
    const auto status = h.runtime.Handle(Configure({Binding(), other}));
    QCOMPARE(status["registered"].toInt(), 1);
    QCOMPARE(status["conflicts"].toArray().size(), 1);
    QVERIFY(h.registrar.callbacks.contains(other.combination));
  }
  void FailedNativeReleaseBlocksRecordingAndLateActivations() {
    Harness h;
    h.runtime.Handle(Configure());
    const auto old_activation = h.registrar.callbacks.value(Binding().combination);
    h.registrar.refuse_clear = true;
    auto status = h.runtime.Handle(Configure({Binding()}, true, true));
    QVERIFY(status["release_failed"].toBool());
    QVERIFY(!status["paused"].toBool()); // No acknowledgement to the editor.
    old_activation();
    QVERIFY(h.api.reads.isEmpty());
    h.registrar.refuse_clear = false;
    status = h.runtime.Handle(Configure({Binding()}, true, true));
    QVERIFY(status["paused"].toBool());
    QVERIFY(h.registrar.callbacks.isEmpty());
  }
  void RetiredNativeCallbacksCannotDispatchAfterReloadOrDestruction() {
    Api api;
    DeviceService devices(&api);
    ShortcutActionService actions(&devices);
    Registrar registrar;
    auto runtime = std::make_unique<ShortcutRuntime>(&registrar, &actions, "account");
    runtime->Handle(Configure());
    const auto callback = registrar.callbacks.value(Binding().combination);
    runtime->Handle(Configure({}));
    callback();
    QVERIFY(api.reads.isEmpty());
    runtime.reset();
    callback();
    QVERIFY(api.reads.isEmpty());
  }
  void InvalidConfigurationIsAtomic_data() {
    QTest::addColumn<QString>("field");
    for (const auto* field :
         {"account", "client", "fraction", "missing", "target", "duplicate", "wrong-type"})
      QTest::newRow(field) << QString(field);
  }
  void InvalidConfigurationIsAtomic() {
    QFETCH(QString, field);
    Harness h;
    h.runtime.Handle(Configure());
    auto request = Configure();
    auto bindings = request["bindings"].toArray();
    auto binding = bindings[0].toObject();
    if (field == "account")
      request["account_id"] = "other";
    else if (field == "client")
      request["client"] = "";
    else if (field == "duplicate")
      bindings.append(binding);
    else {
      if (field == "fraction")
        binding["combination"] = 1.5;
      if (field == "missing")
        binding.remove("combination");
      if (field == "target")
        binding["target"] = QJsonObject{};
      if (field == "wrong-type")
        binding["device_name"] = 123;
      bindings[0] = binding;
    }
    request["bindings"] = bindings;
    QVERIFY(!h.runtime.Handle(request)["ok"].toBool());
    QCOMPARE(h.registrar.callbacks.size(), 1);
    QCOMPARE(h.registrar.registrations, 1);
  }
  void LeasesStopAbandonedForegroundWorkerAndResumeBackgroundRecording() {
    Api api;
    DeviceService devices(&api);
    ShortcutActionService actions(&devices);
    Registrar registrar;
    ShortcutRuntime foreground(&registrar, &actions, "account", nullptr, 60);
    QSignalSpy stopped(&foreground, &ShortcutRuntime::stopRequested);
    foreground.Handle(Configure());
    QTRY_COMPARE_WITH_TIMEOUT(stopped.size(), 1, 2'000);
    QVERIFY(registrar.callbacks.isEmpty());
    ShortcutRuntime background(&registrar, &actions, "account", nullptr, 60);
    background.Handle(Configure({Binding()}, true, true));
    QVERIFY(registrar.callbacks.isEmpty());
    QTRY_COMPARE_WITH_TIMEOUT(background.Status()["registered"].toInt(), 1, 2'000);
    QVERIFY(background.Status()["running"].toBool());
    QVERIFY(!background.Status()["paused"].toBool());
  }
  void MultipleGuiLeasesReleaseIndependently() {
    Harness h;
    h.runtime.Handle(Configure({Binding()}, false, true, "one"));
    h.runtime.Handle(Configure({Binding()}, false, false, "two"));
    QVERIFY(h.runtime.Status()["paused"].toBool());
    h.runtime.Handle({{"command", "release"}, {"client", "one"}});
    QVERIFY(h.runtime.Status()["running"].toBool());
    QCOMPARE(h.runtime.Status()["registered"].toInt(), 1);
    h.runtime.Handle({{"command", "release"}, {"client", "two"}});
    QVERIFY(!h.runtime.Status()["running"].toBool());
  }
  void UnconfiguredWorkerAlsoExpiresAndShutdownRejectsReconfiguration() {
    Harness h;
    h.runtime.Handle({{"command", "release"}, {"client", "never-configured"}});
    QVERIFY(!h.runtime.Status()["running"].toBool());
    QVERIFY(!h.runtime.Handle(Configure())["ok"].toBool());
  }
  void ErrorsTimeoutsInlineCompletionAndServiceDestruction() {
    Api api;
    DeviceService devices(&api);
    auto actions = std::make_unique<ShortcutActionService>(&devices, nullptr, 10);
    QSignalSpy result(actions.get(), &ShortcutActionService::finished);
    actions->Activate(Target());
    const auto stale = api.reads[0].handler;
    QTRY_COMPARE(result.size(), 1);
    QCOMPARE(result[0][1].toString(), QString("timeout"));
    QVERIFY(!api.reads[0].Send(Device()));
    stale(Device()); // Even a backend ignoring its cancelled context cannot write.
    QVERIFY(api.sent.isEmpty());
    api.immediate = Device();
    api.inline_write = true;
    actions->Activate(Target());
    QCOMPARE(result.size(), 2);
    QCOMPARE(result[1][1].toString(), QString{});
    api.immediate.reset();
    actions->Activate(Target());
    actions.reset();
    QVERIFY(!api.reads.last().Send(Device()));
  }
  void DifferentDevicesCanRunIndependentlyAndErrorsReleaseGuard() {
    Harness h;
    auto second = Target();
    second.device_id = "other";
    h.actions.Activate(Target());
    h.actions.Activate(second);
    QCOMPARE(h.api.reads.size(), 2);
    QVERIFY(h.api.reads[0].Send(std::unexpected(ApiError{ApiErrorKind::Http, "synthetic", 401})));
    h.actions.Activate(Target());
    QCOMPARE(h.api.reads.size(), 3);
  }
  void StopHandlersMayDestroyRuntimeDuringIpcCompletion() {
    Api api;
    DeviceService devices(&api);
    ShortcutActionService actions(&devices);
    Registrar registrar;
    for (const auto& command : {QString("stop"), QString("release")}) {
      auto runtime = std::make_unique<ShortcutRuntime>(&registrar, &actions, "account");
      runtime->Handle(Configure());
      connect(runtime.get(), &ShortcutRuntime::stopRequested, this,
              [&runtime] { runtime.reset(); });
      const auto result = runtime->Handle({{"command", command}, {"client", "gui"}});
      QVERIFY(!runtime);
      QVERIFY(result["ok"].toBool());
      QVERIFY(!result["running"].toBool());
      QVERIFY(registrar.callbacks.isEmpty());
    }
  }
  void MachinePreferencesArePersistentAndExcludedFromAccountBackup() {
    QTemporaryDir directory;
    const auto path = directory.filePath("settings.ini");
    Settings first(path);
    QVERIFY(first.GetShortcutsEnabled());
    QVERIFY(!first.GetShortcutsBackground());
    first.SetShortcutsEnabled(true);
    first.SetShortcutsBackground(true);
    Settings second(path);
    QVERIFY(second.GetShortcutsEnabled());
    QVERIFY(second.GetShortcutsBackground());
    const auto backup = second.ExportSettings("account");
    QVERIFY(backup);
    QVERIFY(!backup->contains("shortcuts"));
    second.SetShortcutsEnabled(false);
    Settings disabled(path);
    QVERIFY(!disabled.GetShortcutsEnabled());
    Settings temporary(nullptr, true);
    QVERIFY(temporary.GetShortcutsEnabled());
    temporary.SetShortcutsEnabled(false);
    QVERIFY(!temporary.GetShortcutsEnabled());
    QVERIFY(!temporary.GetShortcutsBackground());
  }
};
QTEST_GUILESS_MAIN(ShortcutTests)
#include "ShortcutTests.moc"
