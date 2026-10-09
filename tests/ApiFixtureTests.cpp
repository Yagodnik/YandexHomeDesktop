#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <optional>
#include "api/debug/FixtureApi.h"
#include "app/StartupOptions.h"
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
    QCOMPARE((*device)->capabilities.size(), 1);
    QVERIFY(execution->has_value());
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
    api.PerformActions({}, &context, [&](ApiResult<void> result) {
      QVERIFY(!result); QVERIFY(result.error().message.contains("does not simulate")); ++replies;
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

  void DebugSettingsAreIsolatedInMemory() {
    Settings first(nullptr, true);
    first.SetCurrentTheme(1); first.SetTrayModeEnabled(true);
    QCOMPARE(first.GetCurrentTheme(), 1); QVERIFY(first.GetTrayModeEnabled());
    Settings second(nullptr, true);
    QCOMPARE(second.GetCurrentTheme(), 0); QVERIFY(!second.GetTrayModeEnabled());
    first.Reset(); QCOMPARE(first.GetCurrentTheme(), 0); QVERIFY(!first.GetTrayModeEnabled());
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
