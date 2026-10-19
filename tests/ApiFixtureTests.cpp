#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QScopeGuard>
#include <QSet>
#include <optional>
#include "api/debug/FixtureApi.h"
#include "app/common/StartupOptions.h"
#include "utils/Settings.h"
#ifdef YH_DEBUG_FAKE_API
#include "auth/AuthorizationFactory.h"
#include <memory>
#endif

class ApiFixtureTests final : public QObject {
  Q_OBJECT
private:
  static QJsonObject Fixture() {
    QFile file(FIXTURE_PATH);
    if (!file.open(QIODevice::ReadOnly)) { return {}; }
    auto fixture = QJsonDocument::fromJson(file.readAll()).object();
    fixture["latency_ms"] = 0;
    return fixture;
  }
  static bool Write(const QString& path, const QJsonObject& fixture) {
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
      file.write(QJsonDocument(fixture).toJson()) != -1;
  }
private slots:
  void SharedSettingsPreserveTheGuiExecutable() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto previous_format = QSettings::defaultFormat();
    const auto restore_format = qScopeGuard([previous_format] {
      QSettings::setDefaultFormat(previous_format);
    });
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
    QSettings preferences("ArtemYagodnik", "YandexHomeDesktop");
    preferences.setValue("YandexHomeDesktop", "saved-gui-path");
    preferences.sync();
    Settings headless_settings;
    headless_settings.SetRestEnabled(true);
    preferences.sync();
    QCOMPARE(preferences.value("YandexHomeDesktop").toString(), QString("saved-gui-path"));
  }

  void StartupFlagsChooseGuiOrPreserveCliArguments() {
    auto options = ParseStartupOptions({"app", "--fake-api"}, true);
    QVERIFY(options); QVERIFY(options->use_fake_api);
    QCOMPARE(options->fixture_path, QString(":/debug/api.json"));
    QCOMPARE(options->cli_arguments, QStringList{"app"});
    options = ParseStartupOptions({"app", "--fake-api-data", "a file.json", "--fake-api", "--list-devices"}, true);
    QVERIFY(options); QCOMPARE(options->fixture_path, QString("a file.json"));
    QCOMPARE(options->cli_arguments, (QStringList{"app", "--list-devices"}));
    options = ParseStartupOptions({"app", "--fake-api", "--fake-api-data=fixture.json"}, true);
    QVERIFY(options); QCOMPARE(options->fixture_path, QString("fixture.json"));
    QVERIFY(!ParseStartupOptions({"app", "--fake-api"}, false));
    QVERIFY(!ParseStartupOptions({"app", "--fake-api-data=x"}, false));
    QVERIFY(!ParseStartupOptions({"app", "--fake-api-data=x"}, true));
    QVERIFY(!ParseStartupOptions({"app", "--fake-api", "--fake-api-data"}, true));
    QVERIFY(!ParseStartupOptions({"app", "--fake-api", "--fake-api-data", "--list-devices"}, true));
    QVERIFY(!ParseStartupOptions({"app", "--fake-api", "--fake-api-data="}, true));
    options = ParseStartupOptions({"app", "--", "--fake-api"}, false);
    QVERIFY(options); QVERIFY(!options->use_fake_api);
    QCOMPARE(options->cli_arguments, (QStringList{"app", "--", "--fake-api"}));
  }

  void FixtureDeliversAllReadEndpointsAsynchronously() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    const auto path = dir.filePath("fixture.json"); QVERIFY(Write(path, Fixture()));
    FixtureApi api(path); QVERIFY(api.Validate()); QObject context;
    int replies = 0;
    std::optional<ApiResult<UserInfo>> home;
    std::optional<ApiResult<QList<ScenarioObject>>> scenarios;
    std::optional<ApiResult<AccountInfo>> account;
    std::optional<ApiResult<DeviceInfo>> device;
    std::optional<ApiResult<void>> execution;
    api.GetUserInfo(&context, [&](auto result) { home = std::move(result); ++replies; });
    api.GetScenarios(&context, [&](auto result) { scenarios = std::move(result); ++replies; });
    api.LoadData(&context, [&](auto result) { account = std::move(result); ++replies; });
    api.GetDeviceInfo("lamp", &context, [&](auto result) { device = std::move(result); ++replies; });
    api.ExecuteScenario("evening", &context, [&](auto result) { execution = std::move(result); ++replies; });
    QCOMPARE(replies, 0);
    QTRY_COMPARE(replies, 5);
    QVERIFY(home->has_value()); QCOMPARE((*home)->households.size(), 2);
    QVERIFY(scenarios->has_value()); QCOMPARE((*scenarios)->size(), 3);
    QVERIFY(account->has_value()); QCOMPARE((*account)->display_name, QString("Demo User"));
    QVERIFY((*account)->default_avatar_id.isEmpty());
    QCOMPARE((*account)->id, QString("demo-user"));
    QVERIFY(device->has_value()); QCOMPARE((*device)->id, QString("lamp"));
    QCOMPARE((*device)->capabilities.size(),
      Fixture()["device_info"].toObject()["lamp"].toObject()["capabilities"].toArray().size());
    QVERIFY(execution->has_value());
  }

  void DemoDevicesCoverEveryControlVariant() {
    QTemporaryDir directory;
    const auto path = directory.filePath("fixture.json");
    QVERIFY(Write(path, Fixture()));
    FixtureApi api(path);
    QVERIFY(api.Validate());
    QObject context;
    std::optional<ApiResult<UserInfo>> home;
    api.GetUserInfo(&context, [&](auto result) { home = std::move(result); });
    QTRY_VERIFY(home.has_value());
    QVERIFY(home->has_value());
    QSet<QString> variants;
    for (const auto& summary : (*home)->devices) {
      std::optional<ApiResult<DeviceInfo>> detail;
      api.GetDeviceInfo(summary.id, &context, [&](auto result) { detail = std::move(result); });
      QTRY_VERIFY(detail.has_value());
      QVERIFY(detail->has_value());
      const auto& device = detail->value();
      QCOMPARE(device.name, summary.name);
      QCOMPARE(device.type, summary.type);
      QVERIFY2(device.capabilities.size() >= 3, qPrintable(summary.id));
      QVERIFY2(!device.properties.isEmpty(), qPrintable(summary.id));
      QCOMPARE(device.capabilities.size(), summary.capabilities.size());
      QCOMPARE(device.properties.size(), summary.properties.size());
      bool in_room = false;
      for (const auto& room : (*home)->rooms) {
        if (room.id == summary.room) {
          QVERIFY(room.devices.contains(summary.id));
          QCOMPARE(room.household_id, summary.household_id);
          in_room = true;
        }
      }
      QVERIFY(in_room);
      for (qsizetype i = 0; i < device.capabilities.size(); ++i) {
        const auto& capability = device.capabilities[i];
        QCOMPARE(CapabilityType::AsString(capability.type), CapabilityType::AsString(summary.capabilities[i].type));
        QCOMPARE(capability.state, summary.capabilities[i].state);
        QCOMPARE(capability.parameters, summary.capabilities[i].parameters);
        if (capability.type == CapabilityType::Range) {
          variants.insert(capability.parameters["random_access"].toBool() ? "slider" : "relative");
        } else if (capability.type == CapabilityType::ColorSetting) {
          const auto& parameters = capability.parameters;
          const auto model = parameters.value("color_model").toString();
          if (!model.isEmpty()) variants.insert(model);
          if (parameters.contains("temperature_k") && model.isEmpty()) variants.insert("temperature-only");
          if (parameters.contains("color_scene") && model.isEmpty()) variants.insert("scene-only");
        } else {
          variants.insert(CapabilityType::AsString(capability.type));
        }
      }
      for (qsizetype i = 0; i < device.properties.size(); ++i) {
        const auto& property = device.properties[i];
        const auto type = PropertyType::AsString(property.type);
        QCOMPARE(type, PropertyType::AsString(summary.properties[i].type));
        QCOMPARE(property.state, summary.properties[i].state);
        QCOMPARE(property.parameters, summary.properties[i].parameters);
        variants.insert(type);
      }
    }
    QCOMPARE(variants, (QSet<QString>{"devices.capabilities.on_off", "devices.capabilities.toggle",
      "devices.capabilities.mode", "devices.capabilities.video_stream", "slider", "relative",
      "rgb", "hsv", "temperature-only", "scene-only", "devices.properties.float", "devices.properties.event"}));
  }

  void ReloadsFileAndReportsFailuresWithoutFallback() {
    QTemporaryDir dir; const auto path = dir.filePath("fixture.json");
    auto fixture = Fixture(); QVERIFY(Write(path, fixture)); FixtureApi api(path); QObject context;
    int replies = 0; QString error;
    api.ExecuteScenario("failure", &context, [&](ApiResult<void> result) {
      if (!result) { error = result.error().message; } ++replies;
    });
    QTRY_COMPARE(replies, 1); QCOMPARE(error, QString("Example fixture error"));
    api.ExecuteScenario("inactive", &context, [&](ApiResult<void> result) {
      QVERIFY(!result); ++replies;
    });
    api.GetDeviceInfo("missing", &context, [&](ApiResult<DeviceInfo> result) {
      QVERIFY(!result); ++replies;
    });
    api.PerformActions({DeviceActionsObject{.id = "missing"}}, &context,
                       [&](ApiResult<void> result) {
                         QVERIFY(!result);
                         QCOMPARE(result.error().kind, ApiErrorKind::InvalidResponse);
                         ++replies;
                       });
    QTRY_COMPARE(replies, 4);
    auto home = fixture["user_info"].toObject();
    home["status"] = "error"; home["message"] = "Simulated outage"; fixture["user_info"] = home;
    QVERIFY(Write(path, fixture));
    api.GetUserInfo(&context, [&](ApiResult<UserInfo> result) {
      QVERIFY(!result); error = result.error().message; ++replies;
    });
    QTRY_COMPARE(replies, 5); QCOMPARE(error, QString("Simulated outage"));
    QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("not json"); file.close();
    QVERIFY(!api.Validate());
    api.GetScenarios(&context, [&](ApiResult<QList<ScenarioObject>> result) {
      QVERIFY(!result); QCOMPARE(result.error().kind, ApiErrorKind::InvalidResponse); ++replies;
    });
    QTRY_COMPARE(replies, 6);
    FixtureApi missing(dir.filePath("missing.json")); QVERIFY(!missing.Validate());
  }

  void ValidatesFixtureShapeAndScopesCallbacks() {
    QTemporaryDir dir; const auto path = dir.filePath("fixture.json");
    auto fixture = Fixture(); fixture["latency_ms"] = -1;
    QVERIFY(Write(path, fixture)); FixtureApi api(path); QVERIFY(!api.Validate());
    fixture = Fixture(); auto home = fixture["user_info"].toObject();
    home["scenarios"] = "not an array"; fixture["user_info"] = home;
    QVERIFY(Write(path, fixture)); QVERIFY(!api.Validate());
    fixture = Fixture(); home = fixture["user_info"].toObject();
    auto devices = home["devices"].toArray(); auto device = devices[0].toObject();
    auto capabilities = device["capabilities"].toArray(); auto capability = capabilities[0].toObject();
    capability.remove("retrievable"); capabilities[0] = capability; device["capabilities"] = capabilities;
    devices[0] = device; home["devices"] = devices; fixture["user_info"] = home;
    QVERIFY(Write(path, fixture)); QVERIFY(!api.Validate());
    QVERIFY(Write(path, Fixture()));
    auto* context = new QObject; bool delivered = false;
    api.GetUserInfo(context, [&](ApiResult<UserInfo>) { delivered = true; });
    delete context; QTest::qWait(20); QVERIFY(!delivered);
  }

  void DeviceActionsSucceedAndSurviveRefresh() {
    QTemporaryDir dir;
    const auto path = dir.filePath("fixture.json");
    auto fixture = Fixture();
    QVERIFY(Write(path, fixture));
    FixtureApi api(path);
    QObject context;
    const auto details = fixture["device_info"].toObject();
    for (auto it = details.begin(); it != details.end(); ++it) {
      const auto info = Serialization::From<DeviceInfo>(it.value().toObject());
      for (const auto& capability : info.capabilities) {
        if (capability.type == CapabilityType::VideoStream)
          continue;
        auto action = capability;
        if (action.type == CapabilityType::OnOff || action.type == CapabilityType::Toggle)
          action.state["value"] = !action.state.value("value").toBool();
        std::optional<ApiResult<void>> response;
        api.PerformActions({DeviceActionsObject{.id = info.id, .actions = {action}}}, &context,
                           [&](auto result) { response = std::move(result); });
        QVERIFY(!response);
        QTRY_VERIFY(response.has_value());
        QVERIFY2(response->has_value(), qPrintable(info.id));
        std::optional<ApiResult<DeviceInfo>> read;
        api.GetDeviceInfo(info.id, &context, [&](auto result) { read = std::move(result); });
        QTRY_VERIFY(read.has_value());
        QVERIFY(read->has_value());
        const auto row = &capability - info.capabilities.constData();
        QCOMPARE(read->value().capabilities[row].state, action.state);
      }
    }
    // Home snapshots reflect the same local state, and a new adapter starts clean.
    std::optional<ApiResult<UserInfo>> home;
    api.GetUserInfo(&context, [&](auto result) { home = std::move(result); });
    QTRY_VERIFY(home.has_value());
    QVERIFY(home->has_value());
    QVERIFY(home->value().devices[0].capabilities[0].state !=
            Serialization::From<UserInfo>(fixture["user_info"].toObject())
                .devices[0]
                .capabilities[0]
                .state);
    fixture["action_errors"] = QJsonObject{{"lamp", "TEST_ERROR"}};
    QVERIFY(Write(path, fixture));
    std::optional<ApiResult<void>> failure;
    api.PerformActions({DeviceActionsObject{.id = "lamp"}}, &context,
                       [&](auto result) { failure = std::move(result); });
    QTRY_VERIFY(failure.has_value());
    QVERIFY(!failure->has_value());
    QCOMPARE(failure->error().message, QString("TEST_ERROR"));
  }

  void LocalVideoStreamAndRelativeActions() {
    QTemporaryDir directory;
    const auto path = directory.filePath("fixture.json");
    QVERIFY(Write(path, Fixture()));
    FixtureApi api(path);
    QObject context;
    std::optional<ApiResult<VideoStream>> stream;
    api.GetVideoStream("camera", &context, [&](auto result) { stream = std::move(result); });
    QVERIFY(!stream);
    QTRY_VERIFY(stream.has_value());
    QVERIFY(stream->has_value());
    QCOMPARE(stream->value().protocol, QString("hls"));
    const QUrl url(stream->value().stream_url);
    QVERIFY(url.isLocalFile());
    QVERIFY(QFile::exists(url.toLocalFile()));
    QVERIFY(QFile::exists(QFileInfo(url.toLocalFile()).dir().filePath("camera.ts")));
    stream.reset();
    api.GetVideoStream("missing", &context, [&](auto result) { stream = std::move(result); });
    QTRY_VERIFY(stream.has_value());
    QVERIFY(!stream->has_value());

    // Apply the same relative action twice in one batch and clamp at the maximum.
    const auto details = Fixture()["device_info"].toObject();
    for (auto it = details.begin(); it != details.end(); ++it) {
      const auto info = Serialization::From<DeviceInfo>(it.value().toObject());
      for (qsizetype row = 0; row < info.capabilities.size(); ++row) {
        auto action = info.capabilities[row];
        if (action.type != CapabilityType::Range ||
            action.parameters.value("random_access").toBool())
          continue;
        action.state["relative"] = true;
        action.state["value"] = 1000;
        std::optional<ApiResult<void>> response;
        api.PerformActions({DeviceActionsObject{.id = info.id, .actions = {action, action}}},
                           &context, [&](auto result) { response = std::move(result); });
        QTRY_VERIFY(response.has_value());
        QVERIFY(response->has_value());
        std::optional<ApiResult<DeviceInfo>> read;
        api.GetDeviceInfo(info.id, &context, [&](auto result) { read = std::move(result); });
        QTRY_VERIFY(read.has_value());
        QVERIFY(read->has_value());
        QCOMPARE(read->value().capabilities[row].state.value("value"),
                 action.parameters.value("range").toMap().value("max"));
        QVERIFY(!read->value().capabilities[row].state.contains("relative"));
      }
    }
    bool delivered = false;
    auto* owner = new QObject;
    api.GetVideoStream("camera", owner, [&](auto) { delivered = true; });
    delete owner;
    QTest::qWait(10);
    QVERIFY(!delivered);
  }

  void DebugSettingsAreIsolatedInMemory() {
    Settings first(nullptr, true);
    first.SetCurrentTheme(1); first.SetTrayModeEnabled(true);
    first.SetShowKeybinds(false);
    QCOMPARE(first.GetCurrentTheme(), 1); QVERIFY(first.GetTrayModeEnabled());
    Settings second(nullptr, true);
    QCOMPARE(second.GetCurrentTheme(), 0); QVERIFY(!second.GetTrayModeEnabled());
    QVERIFY(second.GetShowKeybinds());
    QVERIFY(!first.GetShowKeybinds());
    first.Reset(); QCOMPARE(first.GetCurrentTheme(), 0); QVERIFY(!first.GetTrayModeEnabled());
    QVERIFY(first.GetShowKeybinds());
  }

  void KeybindVisibilityPersistsAndResets() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = directory.filePath("settings.ini");
    {
      Settings settings(path);
      QVERIFY(settings.GetShowKeybinds());
      QSignalSpy changed(&settings, &Settings::showKeybindsChanged);
      settings.SetShowKeybinds(false);
      settings.SetShowKeybinds(false);
      QCOMPARE(changed.size(), 1);
    }
    Settings restored(path);
    QVERIFY(!restored.GetShowKeybinds());
    QSignalSpy changed(&restored, &Settings::showKeybindsChanged);
    restored.Reset();
    QVERIFY(restored.GetShowKeybinds());
    QCOMPARE(changed.size(), 1);
    Settings reset(path);
    QVERIFY(reset.GetShowKeybinds());
  }

#ifdef YH_DEBUG_FAKE_API
  void FixtureSignInAndLogoutStayLocal() {
    std::unique_ptr<IAuthorizationService> auth(CreateAuthorizationService(AuthorizationMode::Fixture, nullptr));
    QSignalSpy authorized(auth.get(), &IAuthorizationService::authorized);
    QSignalSpy unauthorized(auth.get(), &IAuthorizationService::unauthorized);
    QSignalSpy logged_out(auth.get(), &IAuthorizationService::logoutFinished);
    QVERIFY(auth->IsAuthorized()); QVERIFY(!auth->GetToken());
    auth->AttemptLocalAuthorization(); QCOMPARE(authorized.size(), 0);
    QTRY_COMPARE(authorized.size(), 1);
    auth->Logout(); QVERIFY(!auth->IsAuthorized()); QTRY_COMPARE(logged_out.size(), 1);
    auth->AttemptLocalAuthorization(); QTRY_COMPARE(unauthorized.size(), 1);
    auth->AttemptAuthorization(); QTRY_COMPARE(authorized.size(), 2); QVERIFY(auth->IsAuthorized());
  }

  void FixtureLogoutInvalidatesQueuedLoginAndBlocksOverlappingCommands() {
    std::unique_ptr<IAuthorizationService> auth(CreateAuthorizationService(AuthorizationMode::Fixture, nullptr));
    QSignalSpy authorized(auth.get(), &IAuthorizationService::authorized);
    QSignalSpy logout(auth.get(), &IAuthorizationService::logout);
    QSignalSpy finished(auth.get(), &IAuthorizationService::logoutFinished);
    auth->AttemptLocalAuthorization();
    auth->Logout();
    auth->Logout();
    auth->AttemptAuthorization();
    QCOMPARE(logout.size(), 1);
    QTRY_COMPARE(finished.size(), 1);
    QCOMPARE(authorized.size(), 0);
    QVERIFY(!auth->IsAuthorized());
    auth->AttemptAuthorization();
    QTRY_COMPARE(authorized.size(), 1);
  }
#endif
};
QTEST_GUILESS_MAIN(ApiFixtureTests)
#include "ApiFixtureTests.moc"
