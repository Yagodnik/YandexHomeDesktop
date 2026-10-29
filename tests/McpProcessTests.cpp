#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QUuid>
#include "app/mcp/McpGuiHost.h"
#include "app/mcp/McpConfiguration.h"
#include "worker/WorkerControlClient.h"
#include "services/DeviceService.h"
#include "services/HomeService.h"
#include "services/ScenarioService.h"
#include "utils/Settings.h"
#include "models/McpViewModel.h"
#include "worker/WorkerControlServer.h"

namespace {
QString worker_path;
QString cli_path;
QByteArray worker_token;
class FakeApi final : public IHomeApi {
public:
  QList<DeviceActionsObject> actions;
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo> handler) override {
    UserInfo data{}; data.status = Status::Ok;
    RoomObject room{}; room.id = "living"; room.name = "Living room"; data.rooms.append(room);
    DeviceObject lamp{}; lamp.id = "lamp"; lamp.name = "Lamp"; lamp.room = room.id; data.devices.append(lamp);
    handler(data);
  }
  void GetDeviceInfo(const QString& id, QObject*, ApiResultHandler<DeviceInfo> handler) override {
    DeviceInfo device{}; device.id = id; device.status = Status::Ok;
    CapabilityObject capability{}; capability.type = CapabilityType::OnOff;
    capability.state = {{"instance", "on"}, {"value", false}};
    device.capabilities.append(capability); handler(device);
  }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>> handler) override { handler(QList<ScenarioObject>{}); }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void> handler) override { handler(ApiResult<void>{}); }
  void PerformActions(const QList<DeviceActionsObject>& value, QObject*, ApiResultHandler<void> handler) override {
    actions = value; handler(ApiResult<void>{});
  }
};

QJsonObject Post(quint16 port, const QJsonObject& payload, bool authenticate = true,
                 const QByteArray& origin = {}, int* status = nullptr) {
  QNetworkAccessManager manager;
  QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1/mcp").arg(port)));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  if (authenticate) request.setRawHeader("Authorization", "Bearer " + worker_token);
  if (!origin.isEmpty()) request.setRawHeader("Origin", origin);
  auto* reply = manager.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
  QEventLoop loop;
  QTimer timer; timer.setSingleShot(true);
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
  timer.start(2'000); loop.exec();
  if (status) *status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const auto body = reply->readAll();
  return QJsonDocument::fromJson(body).object();
}
}

class McpProcessTests final : public QObject {
  Q_OBJECT
private slots:
  void PortPreferencesPersist() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    const auto path = dir.filePath("settings.ini");
    Settings settings(path);
    QCOMPARE(settings.GetMcpPort(), quint16(8767));
    settings.SetMcpPort(9876);
    Settings reopened(path);
    QCOMPARE(reopened.GetMcpPort(), quint16(9876));
  }
  void ModelRestrictsPortChangesWhileRunning() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    const auto endpoint = "yh-mcp-model-" + QUuid::createUuid().toString(QUuid::Id128);
    WorkerControlServer server;
    bool running = false;
    server.SetHandler([&](const QJsonObject&) { return QJsonObject{{"ok", true}, {"running", running}}; });
    QVERIFY(server.Start(endpoint));
    Settings settings(dir.filePath("settings.ini"));
    McpViewModel model({endpoint, {}, {}, "stop"}, &settings);
    model.SetPort(9876);
    QCOMPARE(model.Port(), 9876);
    model.SetPort(65536);
    QCOMPARE(model.Port(), 9876);
    running = true;
    model.Refresh();
    QTRY_VERIFY(model.IsRunning());
    model.SetPort(9877);
    QCOMPARE(model.Port(), 9876);
  }
#if defined(YH_MCP_CLI_TEST) && defined(YH_DEBUG_FAKE_API)
  void CliControlsWorker() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    StartupOptions options; options.use_fake_api = true; options.fixture_path = dir.filePath("fixture.json");
    QTcpServer probe; QVERIFY(probe.listen(QHostAddress::LocalHost, 0));
    options.mcp_port = probe.serverPort(); probe.close();
    const auto configuration = MakeMcpConfiguration(options);
    FakeApi api; HomeService home(&api); DeviceService devices(&api); ScenarioService scenarios(&api);
    quint16 port = 8767;
    McpGuiHost gui(configuration.gui_endpoint, &home, &devices, &scenarios, [] { return true; },
                   [&] { return port; }, [&](quint16 value) { port = value; });
    QVERIFY(gui.Start());
    const auto run = [&](const QString& command) {
      QProcess process;
      QStringList args{"--fake-api", "--fake-api-data", options.fixture_path, command, "--json"};
      if (command == "--enable-mcp") args << "--mcp-port" << QString::number(*options.mcp_port);
      process.start(cli_path, args);
      QEventLoop loop;
      QTimer deadline; deadline.setSingleShot(true);
      connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);
      connect(&deadline, &QTimer::timeout, &loop, &QEventLoop::quit);
      deadline.start(15'000); loop.exec();
      if (process.state() != QProcess::NotRunning || process.exitCode() != 0) return QJsonObject{};
      return QJsonDocument::fromJson(process.readAllStandardOutput()).object();
    };
    QVERIFY(run("--enable-mcp")["running"].toBool());
    QCOMPARE(port, *options.mcp_port);
    QVERIFY(run("--status-mcp")["running"].toBool());
    const auto disabled = run("--disable-mcp");
    QVERIFY(disabled["ok"].toBool());
    QVERIFY(!disabled["running"].toBool());
  }
#endif
  void ManagedWorkerLifecycle() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    StartupOptions options; options.use_fake_api = true; options.fixture_path = dir.filePath("fixture.json");
    auto configuration = MakeMcpConfiguration(options);
    configuration.worker.executable = worker_path;
    FakeApi api; HomeService home(&api); DeviceService devices(&api); ScenarioService scenarios(&api);
    QTcpServer probe; QVERIFY(probe.listen(QHostAddress::LocalHost, 0));
    auto port = probe.serverPort(); probe.close();
    McpGuiHost gui(configuration.gui_endpoint, &home, &devices, &scenarios, [] { return true; },
                   [&] { return port; }, [&](quint16 value) { port = value; });
    QVERIFY(gui.Start());
    WorkerControlClient client(configuration.worker);
    QSignalSpy finished(&client, &WorkerControlClient::finished);
    client.Request(WorkerControlClient::Command::Start, {}, {}, 10'000);
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 10'000);
    QVERIFY(finished.takeFirst().at(0).toJsonObject()["running"].toBool());
    client.Request(WorkerControlClient::Command::Stop, {}, {}, 10'000);
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 10'000);
    const auto stopped = finished.takeFirst().at(0).toJsonObject();
    QVERIFY(stopped["ok"].toBool());
    QVERIFY(!stopped["running"].toBool());
  }
  void WorkerServesToolsThroughGui() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    StartupOptions options; options.use_fake_api = true; options.fixture_path = dir.filePath("fixture.json");
    auto configuration = MakeMcpConfiguration(options);
    FakeApi api; HomeService home(&api); DeviceService devices(&api); ScenarioService scenarios(&api);
    bool authorized = true;
    QTcpServer probe; QVERIFY(probe.listen(QHostAddress::LocalHost, 0));
    auto port = probe.serverPort(); probe.close();
    McpGuiHost gui(configuration.gui_endpoint, &home, &devices, &scenarios, [&] { return authorized; },
                   [&] { return port; }, [&](quint16 value) { port = value; });
    QVERIFY(gui.Start());
    QProcess process;
    process.start(worker_path, configuration.worker.arguments);
    QVERIFY(process.waitForStarted());
    QTest::qWait(1000);
    QVERIFY2(process.state() == QProcess::Running, process.readAllStandardError().constData());
    WorkerControlClient control(configuration.worker);
    QEventLoop status_loop;
    QJsonObject status;
    connect(&control, &WorkerControlClient::finished, &status_loop, [&](const QJsonObject& result) {
      status = result; status_loop.quit();
    });
    QTimer::singleShot(3'000, &status_loop, &QEventLoop::quit);
    control.Request(WorkerControlClient::Command::Status);
    status_loop.exec();
    QVERIFY(status["running"].toBool());
    worker_token = status["token"].toString().toLatin1();
    QVERIFY(!worker_token.isEmpty());
    QJsonObject init;
    QTRY_VERIFY_WITH_TIMEOUT(!(init = Post(port, {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"},
                                                {"params", QJsonObject{{"protocolVersion", "2025-11-25"}}}})).isEmpty(), 5000);
    QCOMPARE(init["result"].toObject()["serverInfo"].toObject()["name"].toString(), "yandex-home");
    int notification_status = 0;
    Post(port, {{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}}, true, {}, &notification_status);
    QCOMPARE(notification_status, 202);
    const auto unauthorized = Post(port, {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "ping"}}, false);
    QCOMPARE(unauthorized["error"].toObject()["code"].toString(), "unauthorized");
    const auto browser = Post(port, {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "ping"}}, true, "https://example.org");
    QCOMPARE(browser["error"].toObject()["code"].toString(), "origin_forbidden");
    const auto tools = Post(port, {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}});
    QCOMPARE(tools["result"].toObject()["tools"].toArray().size(), 5);
    const auto call = [port](const QString& name, const QJsonObject& arguments) {
      return Post(port, {{"jsonrpc", "2.0"}, {"id", 3}, {"method", "tools/call"},
                         {"params", QJsonObject{{"name", name}, {"arguments", arguments}}}});
    };
    const auto list = call("list_devices", {});
    const auto text = list["result"].toObject()["content"].toArray()[0].toObject()["text"].toString();
    const auto data = QJsonDocument::fromJson(text.toUtf8()).object();
    QCOMPARE(data["rooms"].toArray()[0].toObject()["name"].toString(), "Living room");
    const auto action = call("control_device", {{"device_id", "lamp"}, {"capability", "on_off"},
      {"state", QJsonObject{{"instance", "on"}, {"value", true}}}});
    QVERIFY(!action["result"].toObject()["isError"].toBool(true));
    QCOMPARE(api.actions.size(), 1);
    QCOMPARE(api.actions[0].id, "lamp");
    authorized = false;
    gui.Reset();
    const auto denied = call("list_devices", {});
    QVERIFY(denied.contains("error"));
    QTRY_VERIFY_WITH_TIMEOUT(process.state() == QProcess::NotRunning, 8000);
  }
};

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  if (argc < 2) return 2;
  worker_path = QString::fromLocal8Bit(argv[1]);
  QStringList args = app.arguments(); args.removeAt(1);
#ifdef YH_MCP_CLI_TEST
  if (args.size() < 2) return 2;
  cli_path = args.takeAt(1);
#endif
  QList<QByteArray> bytes; for (const auto& arg : args) bytes.append(arg.toLocal8Bit());
  QList<char*> pointers; for (auto& arg : bytes) pointers.append(arg.data());
  int count = pointers.size();
  McpProcessTests tests;
  return QTest::qExec(&tests, count, pointers.data());
}
#include "McpProcessTests.moc"
