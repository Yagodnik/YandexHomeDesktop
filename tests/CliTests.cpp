#include "app/cli/CliApplication.h"
#include "cli/CliProgress.h"
#include "cli/CliRunner.h"
#include "cli/commands/DeviceCommands.h"
#include "iot/core/CapabilityState.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <tuple>
#ifdef Q_OS_MACOS
#include <dispatch/dispatch.h>
#endif

namespace {
template <typename T> struct Reply {
  QPointer<QObject> owner;
  ApiResultHandler<T> handler;
  bool Send(ApiResult<T> result) {
    if (!owner || !handler) {
      return false;
    }
    auto callback = std::move(handler);
    callback(std::move(result));
    return true;
  }
};
class FakeApi final : public IHomeApi, public IAccountApi {
public:
  struct Read : Reply<DeviceInfo> {
    QString id;
  };
  struct Action : Reply<void> {
    QList<DeviceActionsObject> actions;
  };
  struct Scenario : Reply<void> {
    QString id;
  };
  QList<Reply<UserInfo>> homes;
  QList<Reply<QList<ScenarioObject>>> lists;
  QList<Reply<AccountInfo>> accounts;
  QList<Read> reads;
  QList<Action> actions;
  QList<Scenario> scenarios;
  std::optional<DeviceInfo> immediate_device;
  bool immediate_action = false;
  void GetUserInfo(QObject* owner, ApiResultHandler<UserInfo> handler) override {
    homes.append({owner, std::move(handler)});
  }
  void GetScenarios(QObject* owner, ApiResultHandler<QList<ScenarioObject>> handler) override {
    lists.append({owner, std::move(handler)});
  }
  void LoadData(QObject* owner, ApiResultHandler<AccountInfo> handler) override {
    accounts.append({owner, std::move(handler)});
  }
  void GetDeviceInfo(const QString& id, QObject* owner,
                     ApiResultHandler<DeviceInfo> handler) override {
    if (immediate_device) {
      handler(*immediate_device);
    } else {
      reads.append({{owner, std::move(handler)}, id});
    }
  }
  void PerformActions(const QList<DeviceActionsObject>& commands, QObject* owner,
                      ApiResultHandler<void> handler) override {
    if (immediate_action) {
      handler(ApiResult<void>{});
    } else {
      actions.append({{owner, std::move(handler)}, commands});
    }
  }
  void ExecuteScenario(const QString& id, QObject* owner, ApiResultHandler<void> handler) override {
    scenarios.append({{owner, std::move(handler)}, id});
  }
};
CapabilityObject Capability(CapabilityType type, QVariantMap parameters = {}) {
  CapabilityObject result{};
  result.type = type;
  result.retrievable = true;
  result.parameters = std::move(parameters);
  return result;
}
DeviceInfo Device(QList<CapabilityObject> capabilities = {}) {
  DeviceInfo result{};
  result.id = "lamp";
  result.name = "Lamp";
  result.type = "devices.types.light";
  result.state = DeviceState::Online;
  result.status = Status::Ok;
  result.capabilities = std::move(capabilities);
  return result;
}
UserInfo Home() {
  UserInfo result{};
  result.status = Status::Ok;
  HouseholdObject house{}, cabin{};
  house.id = "home";
  house.name = "Home";
  cabin.id = "cabin";
  cabin.name = "Cabin";
  result.households = {house, cabin};
  DeviceObject a{}, b{};
  a.id = "lamp";
  a.name = "Lamp";
  a.household_id = "home";
  b.id = "cabin-lamp";
  b.name = "Lamp";
  b.household_id = "cabin";
  result.devices = {a, b};
  return result;
}
ScenarioObject Scenario(const QString& id, const QString& name, bool active) {
  ScenarioObject result{};
  result.id = id;
  result.name = name;
  result.is_active = active;
  return result;
}
struct Harness {
  FakeApi api;
  HomeService home{&api};
  DeviceService devices{&api};
  ScenarioService scenarios{&api};
  AccountService account{&api};
  QList<Reply<void>> resets;
  QByteArray output;
  bool stderr_stream = false;
  int code = -1, completions = 0;
  CliRunner runner{{&home, &devices, &scenarios, &account,
                    [this](QObject* owner, ApiResultHandler<void> handler) {
                      resets.append({owner, std::move(handler)});
                    }},
                   [this](const QByteArray& text, bool error) {
                     output = text;
                     stderr_stream = error;
                   }};
  Harness() {
    QObject::connect(&runner, &CliRunner::finished, &runner, [this](int value) {
      code = value;
      ++completions;
    });
  }
  void Start(QStringList arguments, const CommandRegistry& registry = CommandRegistry::Builtin()) {
    arguments.prepend("app");
    arguments.append("--json");
    auto command = CliCommand::Parse(arguments, registry);
    QVERIFY2(command.has_value(), qPrintable(command ? QString{} : command.error()));
    runner.Start(*command);
  }
  QJsonObject Json() const {
    return QJsonDocument::fromJson(output).object();
  }
  QString Error() const {
    return Json()["error"].toObject()["code"].toString();
  }
};
class EchoCommand final : public ICommand {
public:
  explicit EchoCommand(QString text) : text_(std::move(text)) {}
  bool RequiresAuthorization() const override {
    return false;
  }
  bool UsesServices() const override {
    return false;
  }
  void Execute(CliContext& context) const override {
    context.Complete({{"echo", text_}}, text_);
  }

private:
  QString text_;
};
class CustomRules final : public Iot::ICapabilityRules {
public:
  mutable int values = 0, supported = 0, validated = 0;
  CapabilityType Type() const override {
    return CapabilityType::Toggle;
  }
  Iot::ValidationResult ValidateValue(const QVariantMap&) const override {
    ++values;
    return {};
  }
  bool Supports(const CapabilityObject&, const QVariantMap&) const override {
    ++supported;
    return true;
  }
  Iot::ValidationResult ValidateDevice(const QVariantMap&, const QVariantMap&) const override {
    ++validated;
    return {};
  }
};
class CustomCapability final : public ICapabilityInput {
public:
  mutable int parsed = 0;
  std::shared_ptr<CustomRules> rules = std::make_shared<CustomRules>();
  std::shared_ptr<const Iot::ICapabilityRules> Rules() const override {
    return rules;
  }
  std::expected<QVariantMap, QString> Parse(const CliArguments& arguments) const override {
    ++parsed;
    return Iot::State::Toggle("mute", arguments.Value("value") == "quiet");
  }
};
} // namespace

class CliTests final : public QObject {
  Q_OBJECT
private slots:
  void NativeMainQueueCallbacksReachTheCli() {
#ifdef Q_OS_MACOS
    // QtKeychain uses this queue to deliver the result of the password prompt.
    // Exercise that handoff without touching the user's Keychain or credentials.
    QEventLoop loop;
    struct Delivery {
      QEventLoop* loop;
      bool delivered = false;
    } delivery{&loop};
    dispatch_async_f(dispatch_get_main_queue(), &delivery, [](void* context) {
      auto* result = static_cast<Delivery*>(context);
      result->delivered = true;
      result->loop->quit();
    });
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    loop.exec();
    QVERIFY2(delivery.delivered, "The CLI event loop did not deliver a native main-queue callback");
#else
    QSKIP("Native main-queue delivery is specific to macOS");
#endif
  }

  void NoProgressIsAGlobalOption() {
    const QList<QStringList> commands{
        {"devices", "list"},
        {"devices", "show", "--id", "lamp"},
        {"devices", "set", "--id", "lamp", "--capability", "on_off", "--value", "on"},
        {"scenarios", "list"},
        {"scenarios", "run", "--id", "evening"},
        {"account", "show"},
        {"reset", "--i-know-what-i-am-doing"},
        {"--list-devices"},
        {"--account-info"},
        {"--help"}};
    for (auto arguments : commands) {
      arguments.prepend("app");
      const auto normal = CliCommand::Parse(arguments);
      QVERIFY(normal);
      QVERIFY(!normal->no_progress);
      arguments.append("--no-progress");
      const auto quiet = CliCommand::Parse(arguments);
      QVERIFY2(quiet.has_value(), qPrintable(quiet ? QString{} : quiet.error()));
      QVERIFY(quiet->no_progress);
    }
    QVERIFY(CliCommand::Help("app").contains("--no-progress"));
    QVERIFY(!CliCommand::Parse({"app", "devices", "list", "--no-progress", "--no-progress"}));
  }

  void ProgressPreservesAutomationOutput_data() {
    QTest::addColumn<bool>("terminal");
    QTest::addColumn<bool>("json");
    QTest::addColumn<bool>("no_progress");
    for (const bool terminal : {false, true}) {
      for (const bool json : {false, true}) {
        for (const bool no_progress : {false, true}) {
          const auto name =
              QString("terminal=%1,json=%2,disabled=%3").arg(terminal).arg(json).arg(no_progress);
          QTest::newRow(qPrintable(name)) << terminal << json << no_progress;
        }
      }
    }
  }

  void ProgressPreservesAutomationOutput() {
    QFETCH(bool, terminal);
    QFETCH(bool, json);
    QFETCH(bool, no_progress);
    CliCommand command;
    command.json = json;
    command.no_progress = no_progress;
    QList<QByteArray> writes;
    {
      CliProgress progress(CliProgress::EnabledFor(command, terminal),
                           [&writes](const QByteArray& text) { writes.append(text); });
      progress.SetMessage("Waiting for sign-in");
      QVERIFY(QMetaObject::invokeMethod(&progress, "Advance", Qt::DirectConnection));
      progress.SetMessage("Executing");
    }
    if (terminal && !json && !no_progress) {
      QVERIFY(!writes.isEmpty());
      QVERIFY(writes.first().contains("Waiting for sign-in"));
      QVERIFY(writes[2].contains("Executing"));
      QVERIFY(writes.last().trimmed().isEmpty());
    } else {
      QVERIFY(writes.isEmpty());
    }
  }

  void ProgressStopsBeforeResults_data() {
    QTest::addColumn<int>("exit_code");
    QTest::newRow("success") << int(CliContext::Success);
    QTest::newRow("api-failure") << int(CliContext::RequestFailed);
    QTest::newRow("timeout") << int(CliContext::Timeout);
  }

  void ProgressStopsBeforeResults() {
    QFETCH(int, exit_code);
    FakeApi api;
    DeviceService devices(&api);
    QList<QPair<QByteArray, bool>> writes;
    CliProgress progress(true, [&writes](const QByteArray& text) { writes.append({text, true}); });
    CliRunner runner({nullptr, &devices, nullptr, nullptr, {}},
                     [&progress, &writes](const QByteArray& text, bool error) {
                       progress.Stop();
                       writes.append({text, error});
                     });
    QSignalSpy finished(&runner, &CliRunner::finished);
    const auto command = CliCommand::Parse({"app", "devices", "show", "--id", "lamp"});
    QVERIFY(command);
    progress.SetMessage("Executing");
    runner.Start(*command);
    if (exit_code == CliContext::Success) {
      QVERIFY(api.reads[0].Send(Device()));
    } else if (exit_code == CliContext::Timeout) {
      QVERIFY(QMetaObject::invokeMethod(&runner, "OnTimeout", Qt::DirectConnection));
    } else {
      QVERIFY(api.reads[0].Send(std::unexpected(ApiError{ApiErrorKind::Network, "example error"})));
    }
    QCOMPARE(finished.size(), 1);
    QCOMPARE(finished.first().first().toInt(), exit_code);
    QCOMPARE(writes.last().second, exit_code != CliContext::Success);
    QVERIFY(writes.last().first.endsWith('\n'));
    QVERIFY(!writes.last().first.contains('\r'));
    QVERIFY(writes[writes.size() - 2].first.trimmed().isEmpty());
    const auto count = writes.size();
    QVERIFY(QMetaObject::invokeMethod(&progress, "Advance", Qt::DirectConnection));
    progress.Stop();
    QCOMPARE(writes.size(), count);
  }

  void RegisteredCommandNeedsNoCentralDispatchChange() {
    CommandRegistry registry;
    registry.AddOption({{"text", "t"}, "Text to echo", "value"});
    registry.AddOption({"echo-old", "Legacy echo", "value"});
    registry.AddOption({"echo-info", "Legacy echo help"});
    registry.Register(
        {{"tools", "echo"},
         "Echo a value",
         {"text"},
         [](const CliArguments& args) -> std::expected<std::shared_ptr<const ICommand>, QString> {
           return std::make_shared<EchoCommand>(args.Value("text"));
         }});
    registry.RegisterLegacy({"echo-old", {"tools", "echo"}, "text", {}, {"text"}, "echo-info"});
    const auto help = CliCommand::Help("app", registry);
    QVERIFY(help.contains("tools echo"));
    QVERIFY(help.contains("Echo a value"));
    Harness h;
    h.Start({"tools", "echo", "-t", "hello"}, registry);
    QCOMPARE(h.code, 0);
    QCOMPARE(h.Json()["echo"].toString(), QString("hello"));
    QVERIFY(!h.stderr_stream);
    QCOMPARE(h.completions, 1);
    QVERIFY(h.api.reads.isEmpty());
    QVERIFY(h.api.homes.isEmpty());
    QVERIFY(!CliCommand::Parse({"app", "tools", "echo", "--text", "a", "-t", "b"}, registry));
    h.Start({"--echo-old", "legacy"}, registry);
    QCOMPARE(h.Json()["echo"].toString(), QString("legacy"));
    h.Start({"--echo-old", "ignored", "--echo-info"}, registry);
    QVERIFY(h.Json()["help"].toString().contains("tools echo"));
    QVERIFY(!h.Json()["help"].toString().contains("devices set"));
  }
  void RegisteredInputUsesSharedDomainRules() {
    auto custom = std::make_shared<CustomCapability>();
    CapabilityCommands capabilities;
    capabilities.Register("quiet", custom);
    CommandRegistry registry;
    RegisterDeviceCommands(registry, capabilities);
    Harness h;
    h.Start({"devices", "set", "--id", "lamp", "--capability", "quiet", "--value", "quiet"},
            registry);
    QCOMPARE(custom->parsed, 1);
    QVERIFY(h.api.reads[0].Send(Device({Capability(CapabilityType::Toggle)})));
    QCOMPARE(custom->rules->values, 1);
    QCOMPARE(custom->rules->supported, 1);
    QCOMPARE(custom->rules->validated, 1);
    QCOMPARE(h.api.actions[0].actions[0].actions[0].state.value("value").toBool(), true);
    QVERIFY(h.api.actions[0].Send(ApiResult<void>{}));
    QCOMPARE(h.code, 0);
  }
  void InvalidArguments_data() {
    QTest::addColumn<QStringList>("arguments");
    const QList<QStringList> cases{{},
                                   {"unknown"},
                                   {"devices", "list", "--id", "lamp"},
                                   {"devices", "list", "--json", "-j"},
                                   {"devices", "list", "--timeout", "0"},
                                   {"devices", "list", "--timeout", "3600001"},
                                   {"devices", "list", "--timeout", "nan"},
                                   {"devices", "list", "--household="},
                                   {"reset"},
                                   {"--reset"},
                                   {"devices", "list", "--list-devices"},
                                   {"--list-devices", "--account-info"},
                                   {"devices", "show"},
                                   {"devices", "show", "--id="},
                                   {"devices", "show", "--id", "lamp", "--name", "Lamp"},
                                   {"devices", "show", "--id", "lamp", "--device-id", "lamp"},
                                   {"devices", "show", "--id", "lamp", "--household", "home"},
                                   {"--on_off", "Lamp", "--value", "on", "--capability", "on_off"},
                                   {"--on_off", "Lamp", "--value", "on", "--name", "Lamp"},
                                   {"scenarios", "run", "--name", "Evening", "--household", "home"},
                                   {"devices", "list", "--info"}};
    int i = 0;
    for (const auto& args : cases) {
      QTest::newRow(qPrintable(QString::number(i++))) << args;
    }
  }
  void InvalidArguments() {
    QFETCH(QStringList, arguments);
    arguments.prepend("app");
    const auto result = CliCommand::Parse(arguments);
    QVERIFY(!result);
    QVERIFY(!result.error().isEmpty());
  }
  void InvalidCapabilityValues_data() {
    QTest::addColumn<QStringList>("options");
    const QList<QStringList> cases{
        {"--capability", "unknown", "--value", "on"},
        {"--capability", "on_off"},
        {"--capability", "on_off", "--value", "1"},
        {"--capability", "on_off", "--instance", "mute", "--value", "on"},
        {"--capability", "on_off", "--value", "on", "--relative"},
        {"--capability", "toggle", "--value", "off"},
        {"--capability", "range", "--value", "50"},
        {"--capability", "range", "--instance", "brightness", "--value", "nan"},
        {"--capability", "range", "--instance", "brightness", "--value", "inf"},
        {"--capability", "range", "--instance", "brightness", "--value", "1e999"},
        {"--capability", "mode", "--instance", "work", "--value="},
        {"--capability", "color_setting", "--instance", "rgb", "--value", "16777216"},
        {"--capability", "color_setting", "--instance", "rgb", "--value", "-1"},
        {"--capability", "color_setting", "--instance", "rgb", "--value", "1.5"},
        {"--capability", "color_setting", "--instance", "rgb", "--value", "#bad"},
        {"--capability", "color_setting", "--instance", "temperature_k", "--value", "0"},
        {"--capability", "color_setting", "--instance", "scene", "--value="},
        {"--capability", "color_setting", "--instance", "hsv", "--value", "[]"},
        {"--capability", "color_setting", "--instance", "hsv", "--value",
         "{\"h\":361,\"s\":1,\"v\":1}"},
        {"--capability", "color_setting", "--instance", "hsv", "--value",
         "{\"h\":1,\"s\":101,\"v\":1}"},
        {"--capability", "color_setting", "--instance", "hsv", "--value",
         "{\"h\":1,\"s\":1,\"v\":-1}"},
        {"--capability", "color_setting", "--instance", "hsv", "--value",
         "{\"h\":true,\"s\":1,\"v\":1}"},
        {"--capability", "color_setting", "--instance", "hsv", "--value", "{\"h\":1,\"s\":1}"},
        {"--capability", "color_setting", "--instance", "hsv", "--value",
         "{\"h\":1,\"s\":1,\"v\":1,\"x\":1}"}};
    int i = 0;
    for (auto args : cases) {
      QTest::newRow(qPrintable(QString::number(i++))) << args;
    }
  }
  void InvalidCapabilityValues() {
    QFETCH(QStringList, options);
    QStringList args{"app", "devices", "set", "--id", "lamp"};
    args += options;
    QVERIFY(!CliCommand::Parse(args));
  }
  void LegacyCommandsAndHelpRemainUsable() {
    Harness h;
    h.Start({"--on_off", "Lamp", "--value", "on", "--household", "home"});
    QVERIFY(h.api.homes[0].Send(Home()));
    QVERIFY(h.api.reads[0].Send(Device({Capability(CapabilityType::OnOff)})));
    QCOMPARE(h.api.actions[0].actions[0].actions[0].state,
             (QVariantMap{{"instance", "on"}, {"value", true}}));
    QVERIFY(h.api.actions[0].Send(ApiResult<void>{}));
    QCOMPARE(h.code, 0);
    for (const QStringList args : {QStringList{"app", "--help"}, QStringList{"app", "--help-all"},
                                   QStringList{"app", "--on_off", "Lamp", "--info"}}) {
      auto parsed = CliCommand::Parse(args);
      QVERIFY(parsed);
      QVERIFY(!parsed->operation->RequiresAuthorization());
      QVERIFY(!parsed->operation->UsesServices());
      CliRunner runner({}, [](const QByteArray& output, bool error) {
        QVERIFY(!output.isEmpty());
        QVERIFY(!error);
      });
      QSignalSpy finished(&runner, &CliRunner::finished);
      runner.Start(*parsed);
      QCOMPARE(finished.count(), 1);
      QCOMPARE(finished[0][0].toInt(), 0);
    }
    QVERIFY(CliCommand::Parse({"app", "--list-devices"}));
    QVERIFY(CliCommand::Parse({"app", "--account-info"}));
    QVERIFY(CliCommand::Parse({"app", "--reset", "--i-know-what-i-am-doing"}));
  }
  void TypedActions_data() {
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("instance");
    QTest::addColumn<QString>("value");
    QTest::addColumn<QVariant>("expected");
    QTest::addColumn<CapabilityObject>("capability");
    QTest::newRow("on_off") << "on_off" << "on" << "on" << QVariant(true)
                            << Capability(CapabilityType::OnOff);
    QTest::newRow("toggle") << "toggle" << "mute" << "off" << QVariant(false)
                            << Capability(CapabilityType::Toggle, {{"instance", "mute"}});
    QTest::newRow("range") << "range" << "brightness" << "50.5" << QVariant(50.5)
                           << Capability(CapabilityType::Range,
                                         {{"instance", "brightness"},
                                          {"range", QVariantMap{{"min", 0}, {"max", 100}}}});
    QTest::newRow("mode") << "mode" << "work_speed" << "auto" << QVariant("auto")
                          << Capability(CapabilityType::Mode,
                                        {{"instance", "work_speed"},
                                         {"modes", QVariantList{QVariantMap{{"value", "auto"}}}}});
    QTest::newRow("rgb") << "color_setting" << "rgb" << "#33aaee" << QVariant(double(0x33aaee))
                         << Capability(CapabilityType::ColorSetting, {{"color_model", "rgb"}});
    QTest::newRow("rgb_hex") << "devices.capabilities.color_setting" << "rgb" << "0x33AAEE"
                             << QVariant(double(0x33aaee))
                             << Capability(CapabilityType::ColorSetting, {{"color_model", "rgb"}});
    QTest::newRow("temperature") << "color_setting" << "temperature_k" << "3000" << QVariant(3000.0)
                                 << Capability(CapabilityType::ColorSetting,
                                               {{"temperature_k",
                                                 QVariantMap{{"min", 2000}, {"max", 6500}}}});
    QTest::newRow("hsv") << "color_setting" << "hsv" << "{\"h\":360,\"s\":100,\"v\":0}"
                         << QVariant(QVariantMap{
                                {"h", qlonglong(360)}, {"s", qlonglong(100)}, {"v", qlonglong(0)}})
                         << Capability(CapabilityType::ColorSetting, {{"color_model", "hsv"}});
    QTest::newRow("scene")
        << "color_setting" << "scene" << "sunrise" << QVariant("sunrise")
        << Capability(CapabilityType::ColorSetting,
                      {{"color_scene",
                        QVariantMap{{"scenes", QVariantList{QVariantMap{{"id", "sunrise"}}}}}}});
  }
  void TypedActions() {
    QFETCH(QString, type);
    QFETCH(QString, instance);
    QFETCH(QString, value);
    QFETCH(QVariant, expected);
    QFETCH(CapabilityObject, capability);
    Harness h;
    h.Start({"devices", "set", "--device-id", "lamp", "--capability", type, "--instance", instance,
             "--value", value});
    QVERIFY(h.api.homes.isEmpty());
    QCOMPARE(h.api.reads[0].id, QString("lamp"));
    QVERIFY(h.api.reads[0].Send(Device({capability})));
    QCOMPARE(h.api.actions.size(), 1);
    const auto& command = h.api.actions[0].actions[0];
    QCOMPARE(command.id, QString("lamp"));
    QCOMPARE(command.actions.size(), 1);
    QCOMPARE(command.actions[0].type, capability.type);
    QCOMPARE(command.actions[0].state.value("instance").toString(), instance);
    QCOMPARE(command.actions[0].state.value("value"), expected);
    QVERIFY(!command.actions[0].state.contains("relative"));
    QVERIFY(h.api.actions[0].Send(ApiResult<void>{}));
    QCOMPARE(h.code, 0);
    QVERIFY(h.Json()["ok"].toBool());
    QVERIFY(!h.stderr_stream);
    QCOMPARE(h.completions, 1);
    QCOMPARE(h.api.reads.size(), 1); // acknowledgement, no hidden readback/retry
  }
  void DeviceMetadataRejectsUnsupportedActions() {
    const QList<QPair<QStringList, CapabilityObject>> cases{
        {{"range", "brightness", "101"},
         Capability(CapabilityType::Range, {{"instance", "brightness"},
                                            {"range", QVariantMap{{"min", 0}, {"max", 100}}}})},
        {{"range", "volume", "5"},
         Capability(CapabilityType::Range, {{"instance", "volume"}, {"random_access", false}})},
        {{"mode", "work", "bad"},
         Capability(
             CapabilityType::Mode,
             {{"instance", "work"}, {"modes", QVariantList{QVariantMap{{"value", "auto"}}}}})},
        {{"color_setting", "scene", "bad"},
         Capability(CapabilityType::ColorSetting,
                    {{"color_scene",
                      QVariantMap{{"scenes", QVariantList{QVariantMap{{"id", "sunrise"}}}}}}})},
        {{"color_setting", "temperature_k", "8000"},
         Capability(CapabilityType::ColorSetting,
                    {{"temperature_k", QVariantMap{{"min", 2000}, {"max", 6500}}}})}};
    for (const auto& [args, capability] : cases) {
      Harness h;
      h.Start({"devices", "set", "--id", "lamp", "--capability", args[0], "--instance", args[1],
               "--value", args[2]});
      QVERIFY(h.api.reads[0].Send(Device({capability})));
      QCOMPARE(h.code, 2);
      QCOMPARE(h.Error(), QString("invalid_value"));
      QVERIFY(h.api.actions.isEmpty());
    }
    Harness relative;
    relative.Start({"devices", "set", "--id", "lamp", "--capability", "range", "--instance",
                    "volume", "--value=-5", "--relative"});
    QVERIFY(relative.api.reads[0].Send(Device(
        {Capability(CapabilityType::Range, {{"instance", "volume"}, {"random_access", false}})})));
    QCOMPARE(relative.api.actions[0].actions[0].actions[0].state,
             (QVariantMap{{"instance", "volume"}, {"value", -5.0}, {"relative", true}}));
    QVERIFY(relative.api.actions[0].Send(ApiResult<void>{}));
    QCOMPARE(relative.code, 0);
    Harness missing;
    missing.Start({"devices", "set", "--id", "lamp", "--capability", "toggle", "--instance", "mute",
                   "--value", "on"});
    QVERIFY(missing.api.reads[0].Send(
        Device({Capability(CapabilityType::Toggle, {{"instance", "pause"}})})));
    QCOMPARE(missing.code, 4);
    QCOMPARE(missing.Error(), QString("capability_not_found"));
    QVERIFY(missing.api.actions.isEmpty());
  }
  void NameResolutionNeverChoosesAnArbitraryDevice() {
    Harness ambiguous;
    ambiguous.Start({"devices", "show", "--name", "Lamp"});
    QVERIFY(ambiguous.api.homes[0].Send(Home()));
    QCOMPARE(ambiguous.code, 2);
    QCOMPARE(ambiguous.Error(), QString("ambiguous_target"));
    QCOMPARE(ambiguous.Json()["error"].toObject()["matches"].toArray().size(), 2);
    QVERIFY(ambiguous.api.reads.isEmpty());
    QVERIFY(ambiguous.api.actions.isEmpty());
    Harness scoped;
    scoped.Start({"devices", "show", "--name", "Lamp", "--household", "home"});
    QVERIFY(scoped.api.homes[0].Send(Home()));
    QCOMPARE(scoped.api.reads[0].id, QString("lamp"));
    QVERIFY(scoped.api.reads[0].Send(Device()));
    QCOMPARE(scoped.code, 0);
    for (const auto& args : {QStringList{"devices", "show", "--name", "lamp"},
                             QStringList{"devices", "list", "--household", "missing"}}) {
      Harness h;
      h.Start(args);
      QVERIFY(h.api.homes[0].Send(Home()));
      QCOMPARE(h.code, 4);
      QVERIFY(h.api.reads.isEmpty());
    }
  }
  void ShowAndListSerializeSnapshotsWithoutChangingGuiState() {
    Harness h;
    h.Start({"devices", "list", "--household", "home"});
    QVERIFY(h.api.homes[0].Send(Home()));
    QCOMPARE(h.Json()["devices"].toArray().size(), 1);
    QCOMPARE(h.home.GetLoadState(), HomeService::LoadState::NotLoaded);
    QVERIFY(h.home.GetCurrentHousehold().isEmpty());
    auto device = Device({Capability(CapabilityType::OnOff)});
    device.capabilities[0].state = {{"instance", "on"}, {"value", false}};
    PropertyObject property{};
    property.type = PropertyType::Float;
    property.retrievable = true;
    property.parameters = {{"instance", "temperature"}, {"unit", "unit.temperature.celsius"}};
    property.state = {{"instance", "temperature"}, {"value", 23.5}};
    device.properties = {property};
    h.Start({"devices", "show", "--id", "lamp"});
    QVERIFY(h.api.reads[0].Send(device));
    const auto snapshot = h.Json()["device"].toObject();
    QCOMPARE(snapshot["id"].toString(), QString("lamp"));
    QCOMPARE(snapshot["state"].toString(), QString("online"));
    QCOMPARE(snapshot["capabilities"].toArray()[0].toObject()["state"].toObject()["value"].toBool(),
             false);
    QCOMPARE(snapshot["properties"].toArray()[0].toObject()["state"].toObject()["value"].toDouble(),
             23.5);
    QCOMPARE(h.completions, 2);
  }
  void ApiErrors_data() {
    QTest::addColumn<ApiErrorKind>("kind");
    QTest::addColumn<int>("status");
    QTest::addColumn<int>("code");
    QTest::newRow("network") << ApiErrorKind::Network << 0 << 1;
    QTest::newRow("401") << ApiErrorKind::Http << 401 << 3;
    QTest::newRow("403") << ApiErrorKind::Http << 403 << 3;
    QTest::newRow("404") << ApiErrorKind::Http << 404 << 4;
    QTest::newRow("timeout") << ApiErrorKind::Timeout << 0 << 5;
    QTest::newRow("invalid") << ApiErrorKind::InvalidResponse << 0 << 1;
  }
  void ApiErrors() {
    QFETCH(ApiErrorKind, kind);
    QFETCH(int, status);
    QFETCH(int, code);
    Harness h;
    h.Start({"devices", "show", "--id", "lamp"});
    QVERIFY(h.api.reads[0].Send(std::unexpected(ApiError{kind, "example error", status})));
    QCOMPARE(h.code, code);
    QVERIFY(h.stderr_stream);
    QCOMPARE(h.completions, 1);
    QVERIFY(!h.Json()["ok"].toBool());
    QCOMPARE(h.Json()["error"].toObject()["http_status"].toInt(), status);
  }
  void InvalidOrAmbiguousDeviceReplyDoesNotSendAnAction() {
    for (const int variant : {0, 1, 2}) {
      Harness h;
      h.Start({"devices", "set", "--id", "lamp", "--capability", "on_off", "--value", "on"});
      auto info = Device({Capability(CapabilityType::OnOff)});
      if (variant == 0) {
        info.id = "other";
      }
      if (variant == 1) {
        info.status = Status::Error;
        info.message = "failed";
      }
      if (variant == 2) {
        info.capabilities.append(info.capabilities[0]);
      }
      QVERIFY(h.api.reads[0].Send(info));
      QVERIFY(h.api.actions.isEmpty());
      QCOMPARE(h.code, variant == 2 ? 2 : 1);
    }
  }
  void TimeoutAndDestructionCancelDelivery() {
    Harness h;
    h.Start({"devices", "set", "--id", "lamp", "--capability", "on_off", "--value", "on"});
    QVERIFY(h.api.reads[0].Send(Device({Capability(CapabilityType::OnOff)})));
    QVERIFY(QMetaObject::invokeMethod(&h.runner, "OnTimeout", Qt::DirectConnection));
    QCOMPARE(h.code, 5);
    QCOMPARE(h.Error(), QString("timeout"));
    QCOMPARE(h.completions, 1);
    QVERIFY(!h.api.actions[0].Send(ApiResult<void>{}));
    // A late reply must not complete the next operation on a reused runner.
    h.Start({"devices", "show", "--id", "lamp"});
    QVERIFY(!h.api.actions[0].Send(ApiResult<void>{}));
    QCOMPARE(h.completions, 1);
    QVERIFY(h.api.reads[1].Send(Device()));
    QCOMPARE(h.completions, 2);
    QCOMPARE(h.code, 0);
    FakeApi api;
    HomeService home(&api);
    DeviceService devices(&api);
    auto runner = std::make_unique<CliRunner>(
        CliServices{&home, &devices, nullptr, nullptr, {}},
        [](const QByteArray&, bool) { QFAIL("Delivery after destruction"); });
    const auto command = CliCommand::Parse({"app", "devices", "show", "--id", "lamp"});
    QVERIFY(command);
    runner->Start(*command);
    runner.reset();
    QVERIFY(!api.reads[0].Send(Device()));
  }
  void SynchronousApisCanCompleteExactlyOnce() {
    Harness h;
    h.api.immediate_device = Device({Capability(CapabilityType::OnOff)});
    h.api.immediate_action = true;
    h.Start({"devices", "set", "--id", "lamp", "--capability", "on_off", "--value", "on"});
    QCOMPARE(h.code, 0);
    QCOMPARE(h.completions, 1);
    QVERIFY(QMetaObject::invokeMethod(&h.runner, "OnTimeout", Qt::DirectConnection));
    QCOMPARE(h.completions, 1);
  }
  void ScenariosResolveValidateAndExecuteWithoutGuiState() {
    const QList<ScenarioObject> list{Scenario("evening", "Evening", true),
                                     Scenario("inactive", "Inactive", false),
                                     Scenario("other", "Evening", true)};
    Harness h;
    h.Start({"scenarios", "list"});
    QVERIFY(h.api.lists[0].Send(list));
    QCOMPARE(h.Json()["scenarios"].toArray().size(), 3);
    QCOMPARE(h.scenarios.GetLoadState(), ScenarioService::LoadState::NotLoaded);
    h.Start({"scenarios", "run", "--id", "evening"});
    QVERIFY(h.api.lists[1].Send(list));
    QCOMPARE(h.api.scenarios[0].id, QString("evening"));
    QVERIFY(!h.scenarios.IsExecuting("evening"));
    QVERIFY(h.api.scenarios[0].Send(ApiResult<void>{}));
    QCOMPARE(h.code, 0);
    for (const auto& [selector, value, code] :
         {std::tuple{"--name", "Evening", 2}, std::tuple{"--id", "missing", 4},
          std::tuple{"--id", "inactive", 2}}) {
      Harness bad;
      bad.Start({"scenarios", "run", selector, value});
      QVERIFY(bad.api.lists[0].Send(list));
      QCOMPARE(bad.code, code);
      QVERIFY(bad.api.scenarios.isEmpty());
    }
    Harness failed;
    failed.Start({"scenarios", "run", "--name", "Inactive"});
    QVERIFY(failed.api.lists[0].Send(QList<ScenarioObject>{Scenario("unique", "Inactive", true)}));
    QVERIFY(
        failed.api.scenarios[0].Send(std::unexpected(ApiError{ApiErrorKind::Service, "failed"})));
    QCOMPARE(failed.code, 1);
  }
  void AccountAndConfirmedResetUseInjectedServices() {
    Harness h;
    h.Start({"account", "show"});
    QVERIFY(h.api.accounts[0].Send(AccountInfo{"Demo", {}, "demo@example.invalid"}));
    QCOMPARE(h.Json()["account"].toObject()["email"].toString(), QString("demo@example.invalid"));
    auto reset = CliCommand::Parse({"app", "reset", "--i-know-what-i-am-doing", "--json"});
    QVERIFY(reset);
    QVERIFY(!reset->operation->RequiresAuthorization());
    h.runner.Start(*reset);
    QCOMPARE(h.resets.size(), 1);
    QVERIFY(h.resets[0].Send(ApiResult<void>{}));
    QCOMPARE(h.code, 0);
    QCOMPARE(h.completions, 2);
    QVERIFY(h.Json()["reset"].toBool());
  }
};
int main(int argc, char* argv[]) {
  PrepareCliApplication();
  QCoreApplication app(argc, argv);
  CliTests tests;
  return QTest::qExec(&tests, argc, argv);
}
#include "CliTests.moc"
