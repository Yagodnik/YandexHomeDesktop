#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QProcess>
#include <QSignalSpy>
#include "app/shortcuts/ShortcutConfiguration.h"
#include "services/ShortcutControlService.h"

namespace {
QString executable;
bool fixtures = false;
struct Worker {
  QTemporaryDir directory;
  StartupOptions options;
  WorkerLaunchConfiguration configuration;
  Worker() {
    const auto path = directory.filePath("fixture with spaces.json");
    QFile::copy(FIXTURE_PATH, path);
    options.use_fake_api = true;
    options.fixture_path = path;
    configuration = ShortcutConfiguration(options);
    configuration.executable = executable;
  }
  ~Worker() {
    Request(WorkerControlClient::Command::Stop);
  }
  QJsonObject Request(WorkerControlClient::Command command, QJsonObject message = {}) {
    WorkerControlClient client(configuration);
    QEventLoop loop;
    QJsonObject result;
    QObject::connect(&client, &WorkerControlClient::finished, &loop, [&](const QJsonObject& value) {
      result = value;
      loop.quit();
    });
    QTimer::singleShot(0, &loop, [&] { client.Request(command, message, {}, 5'000); });
    loop.exec();
    return result;
  }
};
} // namespace

class ShortcutProcessTests final : public QObject {
  Q_OBJECT
private slots:
  void ScenarioBindingExecutesThroughWorkerAndRejectsInactiveScenario() {
    if (!fixtures)
      QSKIP("Fixture worker is available in Debug");
    Worker worker;
    Settings settings(nullptr, true);
    ShortcutControlService control(&settings, worker.configuration);
    control.SetAccountId("demo-user");
    const KeybindTarget target{.action = "run", .scenario_id = "evening"};
    QCOMPARE(
        settings.SaveKeybind("demo-user", target, Qt::Key_F8, "Evening lights", "Run scenario"),
        Settings::KeybindSaveResult::Saved);
    control.SetEnabled(true);
    QTRY_COMPARE_WITH_TIMEOUT(control.GetStatus()["registered"].toInt(), 1, 7'000);
    worker.Request(WorkerControlClient::Command::Send,
                   {{"command", "activate"}, {"combination", int(Qt::Key_F8)}});
    QTRY_COMPARE_WITH_TIMEOUT(
        worker.Request(WorkerControlClient::Command::Status)["completed"].toInt(), 1, 5'000);
    // Every activation loads fresh scenario state from the fixture-backed API.
    QFile file(worker.options.fixture_path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    auto fixture = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    auto user = fixture["user_info"].toObject();
    auto scenarios = user["scenarios"].toArray();
    bool changed = false;
    for (qsizetype i = 0; i < scenarios.size(); ++i) {
      auto scenario = scenarios[i].toObject();
      if (scenario["id"] == "evening") {
        scenario["is_active"] = false;
        scenarios[i] = scenario;
        changed = true;
      }
    }
    QVERIFY(changed);
    user["scenarios"] = scenarios;
    fixture["user_info"] = user;
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(file.write(QJsonDocument(fixture).toJson()) > 0);
    file.close();
    worker.Request(WorkerControlClient::Command::Send,
                   {{"command", "activate"}, {"combination", int(Qt::Key_F8)}});
    QTRY_COMPARE_WITH_TIMEOUT(
        worker.Request(WorkerControlClient::Command::Status)["failed"].toInt(), 1, 5'000);
    QCOMPARE(worker.Request(WorkerControlClient::Command::Status)["action_error"].toString(),
             QString("scenario_inactive"));
    QCOMPARE(settings.SaveKeybind("demo-user", target, 0, "Evening", "Run"),
             Settings::KeybindSaveResult::Saved);
    QTRY_COMPARE_WITH_TIMEOUT(control.GetStatus()["registered"].toInt(), 0, 5'000);
    control.SetEnabled(false);
    QTRY_VERIFY_WITH_TIMEOUT(!control.IsBusy(), 5'000);
    QVERIFY(!worker.Request(WorkerControlClient::Command::Status)["running"].toBool());
  }

  void HelpAndInvalidArgumentsDoNotNeedGuiOrCredentials() {
    for (const auto& arguments : {QStringList{"--help", "--json"}, QStringList{"--unknown"},
                                  QStringList{"--status", "--stop"}}) {
      QProcess process;
      process.start(executable, arguments);
      QVERIFY(process.waitForFinished(5'000));
      const auto result = QJsonDocument::fromJson(process.readAllStandardOutput()).object();
      QCOMPARE(result["ok"].toBool(), arguments.contains("--help"));
    }
    if (!fixtures) {
      QProcess process;
      process.start(executable, {"--fake-api", "--help"});
      QVERIFY(process.waitForFinished(5'000));
      QVERIFY(process.exitCode() != 0);
    }
  }
  void GuiControllerReloadsRecordsAndDisablesRealWorker() {
    if (!fixtures)
      QSKIP("Fixture worker is available in Debug");
    Worker worker;
    Settings settings(nullptr, true);
    ShortcutControlService control(&settings, worker.configuration);
    control.SetAccountId("demo-user");
    const KeybindTarget target{.device_id = "lamp",
                               .capability_type = "devices.capabilities.on_off",
                               .instance = "on",
                               .action = "toggle",
                               .value = {}};
    QCOMPARE(settings.SaveKeybind("demo-user", target,
                                  QKeyCombination(Qt::ControlModifier, Qt::Key_K).toCombined(),
                                  "Lamp", "Power"),
             Settings::KeybindSaveResult::Saved);
    control.SetEnabled(true);
    QTRY_COMPARE_WITH_TIMEOUT(control.GetStatus()["registered"].toInt(), 1, 7'000);
    const auto first_pid = control.GetStatus()["pid"].toInteger();
    QVERIFY(first_pid > 0);
    worker.Request(WorkerControlClient::Command::Send,
                   {{"command", "activate"},
                    {"combination", QKeyCombination(Qt::ControlModifier, Qt::Key_K).toCombined()}});
    QTRY_COMPARE_WITH_TIMEOUT(
        worker.Request(WorkerControlClient::Command::Status)["completed"].toInt(), 1, 5'000);
    QCOMPARE(worker.Request(WorkerControlClient::Command::Status)["failed"].toInt(), 0);
    // Deliberate fixture failures remain available after successful local writes.
    QFile fixture_file(worker.options.fixture_path);
    QVERIFY(fixture_file.open(QIODevice::ReadOnly));
    auto fixture = QJsonDocument::fromJson(fixture_file.readAll()).object();
    fixture_file.close();
    fixture["action_errors"] = QJsonObject{{"lamp", "TEST_ERROR"}};
    QVERIFY(fixture_file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(fixture_file.write(QJsonDocument(fixture).toJson()) > 0);
    fixture_file.close();
    worker.Request(WorkerControlClient::Command::Send,
                   {{"command", "activate"},
                    {"combination", QKeyCombination(Qt::ControlModifier, Qt::Key_K).toCombined()}});
    QTRY_COMPARE_WITH_TIMEOUT(
        worker.Request(WorkerControlClient::Command::Status)["failed"].toInt(), 1, 5'000);
    QCOMPARE(worker.Request(WorkerControlClient::Command::Status)["action_error"].toString(),
             QString("api_error"));
    control.BeginRecording();
    QTRY_VERIFY_WITH_TIMEOUT(control.IsRecordingReady(), 5'000);
    QVERIFY(control.GetStatus()["paused"].toBool());
    QCOMPARE(control.GetStatus()["registered"].toInt(), 0);
    control.EndRecording();
    QTRY_COMPARE_WITH_TIMEOUT(control.GetStatus()["registered"].toInt(), 1, 5'000);
    QCOMPARE(control.GetStatus()["pid"].toInteger(), first_pid);
    QCOMPARE(settings.SaveKeybind("demo-user", target, 0, "Lamp", "Power"),
             Settings::KeybindSaveResult::Saved);
    QTRY_COMPARE_WITH_TIMEOUT(control.GetStatus()["registered"].toInt(), 0, 5'000);
    control.SetEnabled(false);
    QTRY_VERIFY_WITH_TIMEOUT(!control.IsBusy(), 5'000);
    QVERIFY(!control.GetStatus()["running"].toBool());
    QVERIFY(!worker.Request(WorkerControlClient::Command::Status)["running"].toBool());
  }
  void ShutdownDuringStartupAndLogoutCannotLeaveWorkerRunning() {
    if (!fixtures)
      QSKIP("Fixture worker is available in Debug");
    Worker worker;
    Settings settings(nullptr, true);
    ShortcutControlService control(&settings, worker.configuration);
    control.SetAccountId("demo-user");
    control.SetEnabled(true);
    QSignalSpy closed(&control, &ShortcutControlService::shutdownFinished);
    control.Shutdown();
    QTRY_COMPARE_WITH_TIMEOUT(closed.size(), 1, 8'000);
    QVERIFY(!worker.Request(WorkerControlClient::Command::Status)["running"].toBool());
    ShortcutControlService next(&settings, worker.configuration);
    next.SetAccountId("demo-user");
    QTRY_VERIFY_WITH_TIMEOUT(next.GetStatus()["running"].toBool(), 7'000);
    next.SetEnabled(false);
    next.SetAccountId({});
    QTRY_VERIFY_WITH_TIMEOUT(!next.IsBusy(), 5'000);
    QVERIFY(!worker.Request(WorkerControlClient::Command::Status)["running"].toBool());
  }
  void BackgroundWorkerSurvivesGuiAndForegroundWorkerExitsOnRelease() {
    if (!fixtures)
      QSKIP("Fixture worker is available in Debug");
    Worker worker;
    Settings settings(nullptr, true);
    ShortcutControlService control(&settings, worker.configuration);
    control.SetAccountId("demo-user");
    control.SetBackground(true);
    control.SetEnabled(true);
    QTRY_VERIFY_WITH_TIMEOUT(control.GetStatus()["running"].toBool(), 7'000);
    QSignalSpy closed(&control, &ShortcutControlService::shutdownFinished);
    control.BeginRecording();
    QTRY_VERIFY_WITH_TIMEOUT(control.IsRecordingReady(), 5'000);
    control.Shutdown();
    QTRY_COMPARE_WITH_TIMEOUT(closed.size(), 1, 5'000);
    QVERIFY(!worker.Request(WorkerControlClient::Command::Status)["paused"].toBool());
    QVERIFY(worker.Request(WorkerControlClient::Command::Status)["running"].toBool());
    ShortcutControlService foreground(&settings, worker.configuration);
    foreground.SetAccountId("demo-user");
    foreground.SetBackground(false);
    QTRY_VERIFY_WITH_TIMEOUT(foreground.GetStatus()["running"].toBool() && !foreground.IsBusy(),
                             5'000);
    QSignalSpy released(&foreground, &ShortcutControlService::shutdownFinished);
    foreground.Shutdown();
    QTRY_COMPARE_WITH_TIMEOUT(released.size(), 1, 5'000);
    QVERIFY(!worker.Request(WorkerControlClient::Command::Status)["running"].toBool());
  }
  void ConcurrentGuiControllersShareOneWorkerAndIndependentRecordingLeases() {
    if (!fixtures)
      QSKIP("Fixture worker is available in Debug");
    Worker worker;
    Settings settings(nullptr, true);
    settings.SetShortcutsEnabled(true);
    ShortcutControlService first(&settings, worker.configuration),
        second(&settings, worker.configuration);
    first.SetAccountId("demo-user");
    second.SetAccountId("demo-user");
    QTRY_VERIFY_WITH_TIMEOUT(
        first.GetStatus()["running"].toBool() && second.GetStatus()["running"].toBool(), 8'000);
    QCOMPARE(first.GetStatus()["pid"].toInteger(), second.GetStatus()["pid"].toInteger());
    first.BeginRecording();
    second.BeginRecording();
    QTRY_VERIFY_WITH_TIMEOUT(first.IsRecordingReady() && second.IsRecordingReady(), 5'000);
    QSignalSpy released(&first, &ShortcutControlService::shutdownFinished);
    first.Shutdown();
    QTRY_COMPARE_WITH_TIMEOUT(released.size(), 1, 5'000);
    QVERIFY(worker.Request(WorkerControlClient::Command::Status)["paused"].toBool());
    second.EndRecording();
    QTRY_VERIFY_WITH_TIMEOUT(!second.GetStatus()["paused"].toBool(), 5'000);
    second.SetEnabled(false);
    QTRY_VERIFY_WITH_TIMEOUT(!second.IsBusy(), 5'000);
  }
  void NewGuiStopsBackgroundWorkerWhenPreviousDisableWasInterrupted() {
    if (!fixtures)
      QSKIP("Fixture worker is available in Debug");
    Worker worker;
    Settings settings(nullptr, true);
    {
      ShortcutControlService first(&settings, worker.configuration);
      first.SetAccountId("demo-user");
      first.SetBackground(true);
      first.SetEnabled(true);
      QTRY_VERIFY_WITH_TIMEOUT(first.GetStatus()["running"].toBool() && !first.IsBusy(), 7'000);
    }
    settings.SetShortcutsEnabled(false); // Previous GUI died before sending Stop.
    ShortcutControlService next(&settings, worker.configuration);
    QTRY_VERIFY_WITH_TIMEOUT(next.GetStatus().contains("running") && !next.IsBusy(), 5'000);
    QVERIFY(!worker.Request(WorkerControlClient::Command::Status)["running"].toBool());
    QVERIFY(next.IsRecordingReady());
  }
  void AbandonedGuiLeaseEventuallyStopsDetachedWorker() {
    if (!fixtures)
      QSKIP("Fixture worker is available in Debug");
    Worker worker;
    Settings settings(nullptr, true);
    {
      ShortcutControlService control(&settings, worker.configuration);
      control.SetAccountId("demo-user");
      control.SetEnabled(true);
      QTRY_VERIFY_WITH_TIMEOUT(control.GetStatus()["running"].toBool() && !control.IsBusy(), 7'000);
    }
    QTRY_VERIFY_WITH_TIMEOUT(
        !worker.Request(WorkerControlClient::Command::Status)["running"].toBool(), 14'000);
  }
};

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  executable = QString::fromLocal8Bit(argv[1]);
  fixtures = QString::fromLocal8Bit(argv[2]) == "1";
  argc -= 2;
  argv[2] = argv[0];
  ShortcutProcessTests tests;
  return QTest::qExec(&tests, argc, argv + 2);
}
#include "ShortcutProcessTests.moc"
