#include "rest/RestServer.h"
#include "rest/RestEndpoints.h"
#include "services/HomeService.h"
#include "services/DeviceService.h"
#include "services/ScenarioService.h"
#include "services/AccountService.h"
#include "models/RestViewModel.h"
#include "app/common/StartupOptions.h"
#include "app/rest/RestServerConfiguration.h"
#include "app/rest/RestConfiguration.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

namespace {
bool WriteFile(const QString& path, const QByteArray& bytes = {}) {
  QFile file(path);
  return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

QStringList UnresolvedReferences(const QJsonValue& value, const QJsonObject& document) {
  QStringList unresolved;
  if (value.isArray()) {
    for (const auto& item : value.toArray()) {
      unresolved += UnresolvedReferences(item, document);
    }
  } else if (value.isObject()) {
    const auto object = value.toObject();
    if (object.contains("$ref")) {
      const auto reference = object["$ref"].toString();
      QJsonValue target = document;
      if (!reference.startsWith("#/")) {
        unresolved.append(reference);
      } else {
        for (auto segment : reference.sliced(2).split('/')) {
          segment.replace("~1", "/").replace("~0", "~");
          target = target.toObject().value(segment);
        }
        if (!target.isObject()) {
          unresolved.append(reference);
        }
      }
    }
    for (const auto& item : object) {
      unresolved += UnresolvedReferences(item, document);
    }
  }
  return unresolved;
}

// A separate process whose startup and exit are controlled by test files.
// This lets the controller tests observe a live worker independently of its
// listener state, without OAuth, HTTP, or timing-dependent shutdown sleeps.
int RunControlWorker(QCoreApplication& app, const QString& name, const QString& directory) {
  const QDir files(directory);
  if (!WriteFile(files.filePath("pid"), QByteArray::number(app.applicationPid()))) {
    return 2;
  }
  QLocalServer server;
  bool running = true;
  QObject::connect(&server, &QLocalServer::newConnection, &app, [&] {
    while (auto* socket = server.nextPendingConnection()) {
      auto buffer = std::make_shared<QByteArray>();
      QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
      QObject::connect(socket, &QLocalSocket::readyRead, socket, [&, socket, buffer] {
        *buffer += socket->readAll();
        if (!buffer->contains('\n')) {
          return;
        }
        if (QJsonDocument::fromJson(*buffer).object()["command"] == "disable") {
          running = false;
          WriteFile(files.filePath("disabled"));
        }
        socket->write(QJsonDocument(QJsonObject{{"ok", true},
                                                {"enabled", running},
                                                {"running", running},
                                                {"pid", app.applicationPid()},
                                                {"port", 9000}})
                          .toJson(QJsonDocument::Compact) +
                      '\n');
        socket->disconnectFromServer();
      });
    }
  });
  QTimer gates;
  QObject::connect(&gates, &QTimer::timeout, &app, [&] {
    if (QFileInfo::exists(files.filePath("exit"))) {
      app.quit();
      return;
    }
    if (!server.isListening() && QFileInfo::exists(files.filePath("start"))) {
      if (!server.listen(name) || !WriteFile(files.filePath("ready"))) {
        app.exit(2);
      }
    }
  });
  gates.start(10);
  QTimer::singleShot(20'000, &app, &QCoreApplication::quit);
  return app.exec();
}

struct WorkerCleanup {
  QString directory;
  ~WorkerCleanup() {
    WriteFile(QDir(directory).filePath("exit"));
  }
};

class ScriptedApi final : public IHomeApi, public IAccountApi {
public:
  struct Pending {
    QString id;
    QPointer<QObject> context;
    ApiResultHandler<DeviceInfo> handler;
  };

  QList<Pending> reads;
  QList<DeviceActionsObject> actions;
  QStringList executions;
  bool defer = false;
  std::optional<ApiError> error;
  std::optional<DeviceInfo> device_override;

  void GetUserInfo(QObject*, ApiResultHandler<UserInfo> handler) override {
    UserInfo home{};
    home.status = Status::Ok;
    home.devices = {DeviceObject{.id = "lamp", .name = "Lamp", .household_id = "home"}};
    handler(home);
  }

  void GetDeviceInfo(
    const QString& id, QObject* context, ApiResultHandler<DeviceInfo> handler) override {
    if (defer) {
      reads.append({id, context, std::move(handler)});
    } else if (error) {
      handler(std::unexpected(*error));
    } else {
      handler(device_override.value_or(Device(id)));
    }
  }

  static DeviceInfo Device(const QString& id) {
    DeviceInfo device{};
    device.status = Status::Ok;
    device.id = id;
    device.state = DeviceState::Online;
    device.capabilities = {CapabilityObject{.type = CapabilityType::OnOff},
      CapabilityObject{.type = CapabilityType::Range,
        .parameters = {
          {"instance", "brightness"}, {"range", QVariantMap{{"min", 0}, {"max", 100}}}}}};
    return device;
  }

  void PerformActions(
    const QList<DeviceActionsObject>& value, QObject*, ApiResultHandler<void> handler) override {
    actions += value;
    handler(ApiResult<void>{});
  }

  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>> handler) override {
    handler(QList<ScenarioObject>{ScenarioObject{.id = "evening", .is_active = true},
      ScenarioObject{.id = "inactive", .is_active = false}});
  }

  void ExecuteScenario(const QString& id, QObject*, ApiResultHandler<void> handler) override {
    executions.append(id);
    handler(ApiResult<void>{});
  }

  void LoadData(QObject*, ApiResultHandler<AccountInfo> handler) override {
    handler(AccountInfo{"User", {}, "user@example.invalid", "user"});
  }
};

struct Harness {
  ScriptedApi api;
  HomeService home{&api};
  DeviceService devices{&api};
  ScenarioService scenarios{&api};
  AccountService account{&api};
  RestRouter routes = CreateRestRoutes(home, devices, scenarios, account);
  RestServer server{routes, nullptr, 150};
  QNetworkAccessManager network;

  Harness() {
    if (!server.Start(0)) {
      qFatal("Cannot listen on loopback");
    }
  }

  QNetworkReply* Send(const QString& path, const QByteArray& method = "GET",
    const QByteArray& body = {}, const QByteArray& host = {},
    const QByteArray& origin = {}) {
    QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1%2").arg(server.Port()).arg(path)));
    if (!host.isEmpty()) request.setRawHeader("Host", host);
    request.setRawHeader("Content-Type", "application/json");
    if (!origin.isEmpty()) {
      request.setRawHeader("Origin", origin);
    }
    return network.sendCustomRequest(request, method, body);
  }
};

QJsonObject Body(QNetworkReply* reply) {
  return QJsonDocument::fromJson(reply->readAll()).object();
}

int HttpStatus(QNetworkReply* reply) {
  return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}
} // namespace

class RestTests final : public QObject {
  Q_OBJECT
private slots:

  void StartupAndPersistence() {
    for (const auto& command :
      {"--enable-rest", "--disable-rest", "--status-rest", "--serve-rest"}) {
      const auto options = ParseStartupOptions({"app", "--fake-api", command, "--json"}, true);
      QVERIFY(options);
      QVERIFY(options->use_fake_api);
      QVERIFY(options->rest_command != StartupOptions::RestCommand::None);
      QCOMPARE(options->cli_arguments, QStringList({"app", "--json"}));
    }
    QVERIFY(!ParseStartupOptions({"app", "--enable-rest", "--disable-rest"}, true));
    QVERIFY(!ParseStartupOptions({"app", "--status-rest", "--rest-port", "9000"}, true));
    QVERIFY(ParseStartupOptions({"rest", "--rest-port", "9000"}, true, true));
    QVERIFY(!ParseStartupOptions({"cli", "--rest-port", "9000"}, true));
    QVERIFY(!ParseStartupOptions({"app", "--enable-rest", "--rest-port", "0"}, true));
    QVERIFY(!ParseStartupOptions({"app", "--enable-rest", "--rest-port", "65536"}, true));
    QVERIFY(!ParseStartupOptions(
      {"app", "--enable-rest", "--rest-port", "9000", "--rest-port=9001"}, true));
    QVERIFY(!ParseStartupOptions({"app", "--fake-api", "--enable-rest"}, false));
    QTemporaryDir directory;
    Settings settings(directory.filePath("settings.ini"));
    QVERIFY(!settings.GetRestEnabled());
    QCOMPARE(settings.GetRestPort(), quint16(8765));
    settings.SetRestPort(9000);
    settings.SetRestEnabled(true);
    Settings second(directory.filePath("settings.ini"));
    QVERIFY(second.GetRestEnabled());
    QCOMPARE(second.GetRestPort(), quint16(9000));
    Settings temporary(nullptr, true);
    QVERIFY(!temporary.GetRestEnabled());
    QCOMPARE(temporary.GetRestPort(), quint16(8766));
    temporary.SetRestEnabled(true);
    QVERIFY(second.GetRestEnabled());
  }

  void ConfigurationStartupOptionsAndLaunchIdentity() {
    const auto options = ParseStartupOptions(
      {"cli", "--enable-rest", "--rest-config", "config with spaces.json", "--rest-port", "9000"}, true);
    QVERIFY(options);
    QCOMPARE(options->rest_config_file, QFileInfo("config with spaces.json").absoluteFilePath());
    QCOMPARE(options->rest_port, std::optional<quint16>(9000));
    QCOMPARE(options->cli_arguments, QStringList{"cli"});
    QVERIFY(ParseStartupOptions({"rest", "--rest-config=config.json"}, false, true));
    QVERIFY(!ParseStartupOptions({"cli", "--rest-config=config.json", "devices", "list"}, true));
    QVERIFY(!ParseStartupOptions({"rest", "--rest-config="}, true, true));
    QVERIFY(!ParseStartupOptions({"rest", "--rest-config", "--json"}, true, true));
    QVERIFY(!ParseStartupOptions({"rest", "--rest-config=a", "--rest-config=b"}, true, true));
    const auto launch = RestConfiguration(*options);
    QVERIFY(launch.arguments.contains(options->rest_config_file));
    QVERIFY(launch.arguments.contains("--rest-config"));
    QVERIFY(launch.control_name != RestConfiguration(StartupOptions{}).control_name);
    auto other = *options;
    other.rest_config_file += ".other";
    QVERIFY(launch.control_name != RestConfiguration(other).control_name);
  }

  void ConfigurationDefaultsAndExplicitToken() {
    const auto defaults = LoadRestServerConfiguration({});
    QVERIFY(defaults);
    QCOMPARE(defaults->host, QHostAddress(QHostAddress::LocalHost));
    QVERIFY(!defaults->port);
    QVERIFY(defaults->oauth_token.isEmpty());
    QTemporaryDir directory;
    const auto path = directory.filePath("config.json");
    QVERIFY(WriteFile(path, R"({"host":"::1","port":9000,"oauth_token":"synthetic-test-token"})"));
    const auto configuration = LoadRestServerConfiguration(path);
    QVERIFY(configuration);
    QCOMPARE(configuration->host, QHostAddress(QHostAddress::LocalHostIPv6));
    QCOMPARE(configuration->port, std::optional<quint16>(9000));
    QVERIFY(configuration->oauth_token == "synthetic-test-token");
    QCOMPARE(configuration->Url(9001), QString("http://[::1]:9001/v1"));
    QVERIFY(WriteFile(path, R"({"host":"localhost"})"));
    const auto local = LoadRestServerConfiguration(path);
    QVERIFY(local);
    QCOMPARE(local->host, QHostAddress(QHostAddress::LocalHost));
    QVERIFY(!local->port);
    QVERIFY(local->oauth_token.isEmpty());
    QVERIFY(WriteFile(path, "{}"));
    const auto empty = LoadRestServerConfiguration(path);
    QVERIFY(empty);
    QCOMPARE(empty->host, QHostAddress(QHostAddress::LocalHost));
    QVERIFY(!empty->port);
    QVERIFY(empty->oauth_token.isEmpty());
    QVERIFY(WriteFile(path, R"({"port":9000})"));
    const auto port_only = LoadRestServerConfiguration(path);
    QVERIFY(port_only);
    QCOMPARE(port_only->host, QHostAddress(QHostAddress::LocalHost));
    QCOMPARE(port_only->port, std::optional<quint16>(9000));
    QVERIFY(port_only->oauth_token.isEmpty());
    QVERIFY(WriteFile(path, R"({"oauth_token":"synthetic-test-token"})"));
    const auto token_only = LoadRestServerConfiguration(path);
    QVERIFY(token_only);
    QCOMPARE(token_only->host, QHostAddress(QHostAddress::LocalHost));
    QVERIFY(!token_only->port);
    QVERIFY(token_only->oauth_token == "synthetic-test-token");
  }

  void RejectsInvalidConfiguration_data() {
    QTest::addColumn<QByteArray>("bytes");
    QTest::newRow("malformed") << QByteArray("{");
    QTest::newRow("array") << QByteArray("[]");
    QTest::newRow("unknown-field") << QByteArray(R"({"unexpected":"synthetic-test-token"})");
    QTest::newRow("hostname") << QByteArray(R"({"host":"example.invalid"})");
    QTest::newRow("host-type") << QByteArray(R"({"host":123})");
    QTest::newRow("null-host") << QByteArray(R"({"host":null})");
    QTest::newRow("host-url") << QByteArray("{\"host\":\"http://127.0.0.1\"}");
    QTest::newRow("zero-port") << QByteArray(R"({"port":0})");
    QTest::newRow("large-port") << QByteArray(R"({"port":65536})");
    QTest::newRow("fractional-port") << QByteArray(R"({"port":9000.5})");
    QTest::newRow("string-port") << QByteArray(R"({"port":"9000"})");
    QTest::newRow("boolean-port") << QByteArray(R"({"port":true})");
    QTest::newRow("null-port") << QByteArray(R"({"port":null})");
    QTest::newRow("empty-token") << QByteArray(R"({"oauth_token":""})");
    QTest::newRow("token-type") << QByteArray(R"({"oauth_token":123})");
    QTest::newRow("null-token") << QByteArray(R"({"oauth_token":null})");
    QTest::newRow("token-newline") << QByteArray(R"({"oauth_token":"synthetic-test-token\r\n"})");
    QTest::newRow("token-space") << QByteArray(R"({"oauth_token":"synthetic-test-token extra"})");
    QTest::newRow("oversized") << QByteArray(64 * 1024 + 1, ' ');
  }

  void RejectsInvalidConfiguration() {
    QFETCH(QByteArray, bytes);
    QTemporaryDir directory;
    const auto path = directory.filePath("config.json");
    QVERIFY(WriteFile(path, bytes));
    const auto configuration = LoadRestServerConfiguration(path);
    QVERIFY(!configuration);
    QVERIFY(!configuration.error().contains("synthetic-test-token"));
    QVERIFY(!LoadRestServerConfiguration(directory.filePath("absent.json")));
  }

  void ConfiguredHostsValidateTheConnectionDestination_data() {
    QTest::addColumn<QHostAddress>("listen_host");
    QTest::addColumn<QString>("request_host");
    QTest::newRow("ipv4") << QHostAddress(QHostAddress::LocalHost) << QString("127.0.0.1");
    QTest::newRow("wildcard") << QHostAddress(QHostAddress::AnyIPv4) << QString("127.0.0.1");
    QTest::newRow("ipv6") << QHostAddress(QHostAddress::LocalHostIPv6) << QString("::1");
  }

  void ConfiguredHostsValidateTheConnectionDestination() {
    QFETCH(QHostAddress, listen_host);
    QFETCH(QString, request_host);
    Harness h;
    h.server.Stop();
    if (!h.server.Start(0, listen_host)) {
      if (listen_host.protocol() == QAbstractSocket::IPv6Protocol) {
        QSKIP("IPv6 loopback is unavailable");
      }
      QFAIL("Cannot open configured listener");
    }
    QUrl url;
    url.setScheme("http");
    url.setHost(request_host);
    url.setPort(h.server.Port());
    url.setPath("/v1/status");
    QNetworkRequest request(url);
    auto* reply = h.network.get(request);
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(HttpStatus(reply), 200);
    auto specification_url = url;
    specification_url.setPath("/v1/openapi.json");
    reply = h.network.get(QNetworkRequest(specification_url));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(HttpStatus(reply), 200);
    auto expected_server = url;
    expected_server.setPath({});
    QCOMPARE(Body(reply)["servers"].toArray().first().toObject()["url"].toString(),
      expected_server.toString());
    request.setRawHeader("Host", "example.invalid:" + QByteArray::number(h.server.Port()));
    reply = h.network.get(request);
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(HttpStatus(reply), 403);
    request.setRawHeader("Host", "localhost:" + QByteArray::number(h.server.Port()));
    reply = h.network.get(request);
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(HttpStatus(reply), 200);
  }

  void AuthRoutesAndReads() {
    Harness h;
    auto* invalid_host = h.Send("/v1/devices", "GET", {}, "example.invalid");
    QTRY_VERIFY(invalid_host->isFinished());
    QCOMPARE(HttpStatus(invalid_host), 403);
    QVERIFY(!Body(invalid_host)["ok"].toBool());
    auto* browser = h.Send("/v1/status", "GET", {}, {}, "https://example.invalid");
    QTRY_VERIFY(browser->isFinished());
    QCOMPARE(HttpStatus(browser), 403);
    auto* wrong_method = h.Send("/v1/devices", "POST");
    QTRY_VERIFY(wrong_method->isFinished());
    QCOMPARE(HttpStatus(wrong_method), 405);
    auto* missing = h.Send("/other");
    QTRY_VERIFY(missing->isFinished());
    QCOMPARE(HttpStatus(missing), 404);
    for (const auto& route :
      {"/v1/status", "/v1/devices", "/v1/devices/lamp", "/v1/scenarios", "/v1/account"}) {
      auto* reply = h.Send(route);
      QTRY_VERIFY(reply->isFinished());
      QCOMPARE(HttpStatus(reply), 200);
      QCOMPARE(reply->rawHeader("Cache-Control"), QByteArray("no-store"));
      const auto body = Body(reply);
      QVERIFY(body["ok"].toBool());
      QVERIFY(!QJsonDocument(body).toJson().contains("test-access-key"));
    }
    QVERIFY(h.api.actions.isEmpty());
  }

  void OpenApiDocumentMatchesBundledSourceAndRouter() {
    Harness h;
    auto* reply = h.Send("/v1/openapi.json");
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(HttpStatus(reply), 200);
    QCOMPARE(reply->rawHeader("Content-Type"), QByteArray("application/json"));
    QCOMPARE(reply->rawHeader("Cache-Control"), QByteArray("no-store"));
    auto document = Body(reply);
    QCOMPARE(document["openapi"].toString(), QString("3.1.0"));
    QVERIFY(!document.contains("ok"));
    QVERIFY(document["security"].toArray().isEmpty());
    QCOMPARE(document["servers"].toArray().first().toObject()["url"].toString(),
      QString("http://127.0.0.1:%1").arg(h.server.Port()));
    QFile source(OPENAPI_PATH);
    QVERIFY(source.open(QIODevice::ReadOnly));
    const auto committed = QJsonDocument::fromJson(source.readAll()).object();
    auto servers = document["servers"].toArray();
    servers.removeFirst();
    document["servers"] = servers;
    QCOMPARE(document, committed);
    QVERIFY(UnresolvedReferences(document, document).isEmpty());
    const QMap<QString, QHttpServerRequest::Method> methods{
      {"get", QHttpServerRequest::Method::Get}, {"post", QHttpServerRequest::Method::Post},
      {"put", QHttpServerRequest::Method::Put}, {"delete", QHttpServerRequest::Method::Delete},
      {"patch", QHttpServerRequest::Method::Patch}, {"head", QHttpServerRequest::Method::Head},
      {"options", QHttpServerRequest::Method::Options}, {"trace", QHttpServerRequest::Method::Trace}};
    QSet<QString> implemented, documented, operation_ids;
    for (const auto& endpoint : h.routes.Endpoints()) {
      const auto method = methods.key(endpoint.method);
      QVERIFY(!method.isEmpty());
      implemented.insert(method + " " + endpoint.path);
    }
    const auto paths = document["paths"].toObject();
    for (auto path = paths.begin(); path != paths.end(); ++path) {
      const auto item = path->toObject();
      for (auto method = methods.begin(); method != methods.end(); ++method) {
        if (!item.contains(method.key())) {
          continue;
        }
        const auto operation = item[method.key()].toObject();
        const auto id = operation["operationId"].toString();
        QVERIFY(!id.isEmpty());
        QVERIFY(!operation_ids.contains(id));
        operation_ids.insert(id);
        documented.insert(method.key() + " " + path.key());
        QVERIFY(operation["responses"].toObject().contains("200"));
      }
      for (const auto& parameter : item["parameters"].toArray()) {
        const auto object = parameter.toObject();
        QCOMPARE(object["in"].toString(), QString("path"));
        QVERIFY(object["required"].toBool());
        QVERIFY(path.key().contains("{" + object["name"].toString() + "}"));
      }
    }
    QCOMPARE(implemented, documented);
    auto* wrong_method = h.Send("/v1/openapi.json", "POST", "{}");
    QTRY_VERIFY(wrong_method->isFinished());
    QCOMPARE(HttpStatus(wrong_method), 405);
    QVERIFY(!Body(wrong_method)["ok"].toBool());
  }

  void OpenApiActionExamplesReachDeviceService() {
    Harness h;
    auto device = ScriptedApi::Device("lamp");
    device.capabilities = {
      CapabilityObject{.type = CapabilityType::OnOff},
      CapabilityObject{.type = CapabilityType::Range, .parameters = {{"instance", "brightness"}}},
      CapabilityObject{.type = CapabilityType::Range, .parameters = {{"instance", "volume"}}},
      CapabilityObject{.type = CapabilityType::Toggle, .parameters = {{"instance", "mute"}}},
      CapabilityObject{.type = CapabilityType::Mode, .parameters = {{"instance", "work_speed"},
        {"modes", QVariantList{QVariantMap{{"value", "auto"}}}}}},
      CapabilityObject{.type = CapabilityType::ColorSetting, .parameters = {{"color_model", "rgb"},
        {"temperature_k", QVariantMap{{"min", 2000}, {"max", 6500}}},
        {"color_scene", QVariantMap{{"scenes", QVariantList{QVariantMap{{"id", "sunrise"}}}}}}}},
      CapabilityObject{.type = CapabilityType::ColorSetting, .parameters = {{"color_model", "hsv"}}}};
    h.api.device_override = device;
    auto* specification = h.Send("/v1/openapi.json");
    QTRY_VERIFY(specification->isFinished());
    const auto document = Body(specification);
    const auto examples = document["paths"].toObject()["/v1/devices/{device_id}/actions"].toObject()
      ["post"].toObject()["requestBody"].toObject()["content"].toObject()["application/json"].toObject()
      ["examples"].toObject();
    QVERIFY(!examples.isEmpty());
    int sent = 0;
    for (auto example = examples.begin(); example != examples.end(); ++example) {
      const auto action = example->toObject()["value"].toObject();
      auto* reply = h.Send("/v1/devices/lamp/actions", "POST", QJsonDocument(action).toJson());
      QTRY_VERIFY(reply->isFinished());
      QVERIFY2(HttpStatus(reply) == 200, qPrintable(example.key()));
      const auto result = Body(reply);
      QVERIFY(result["ok"].toBool());
      QCOMPARE(result["state"].toObject(), action["state"].toObject());
      QCOMPARE(h.api.actions.size(), ++sent);
    }
  }

  void RejectsNonJsonScenarioRequests() {
    Harness h;
    QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1/v1/scenarios/evening/run").arg(h.server.Port())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    auto* reply = h.network.post(request, "{}");
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(HttpStatus(reply), 415);
    QVERIFY(h.api.executions.isEmpty());
  }

  void TypedValidationAndInlineActions() {
    Harness h;
    const QList<QByteArray> invalid{"not json", "[]", "{}",
      R"({"capability":"unknown","state":{"instance":"on","value":true}})",
      R"({"capability":"on_off","state":{"instance":"on","value":"true"}})",
      R"({"capability":"range","state":{"instance":"brightness","value":101}})",
      R"({"capability":"range","state":{"instance":"brightness","value":10,"relative":"false"}})",
      R"({"capability":"on_off","state":{"instance":"on","value":true,"extra":1}})"};
    for (const auto& body : invalid) {
      auto* reply = h.Send("/v1/devices/lamp/actions", "POST", body);
      QTRY_VERIFY(reply->isFinished());
      QCOMPARE(HttpStatus(reply), 400);
    }
    QVERIFY(h.api.actions.isEmpty());
    auto* missing = h.Send("/v1/devices/lamp/actions", "POST",
      R"({"capability":"toggle","state":{"instance":"mute","value":true}})");
    QTRY_VERIFY(missing->isFinished());
    QCOMPARE(HttpStatus(missing), 404);
    auto* action = h.Send("/v1/devices/lamp/actions", "POST",
      R"({"capability":"range","state":{"instance":"brightness","value":-5,"relative":true}})");
    QTRY_VERIFY(action->isFinished());
    QCOMPARE(HttpStatus(action), 200);
    QCOMPARE(h.api.actions.size(), 1);
    QCOMPARE(h.api.actions[0].id, QString("lamp"));
    const auto state = h.api.actions[0].actions[0].state;
    QCOMPARE(state["value"].toDouble(), -5);
    QCOMPARE(state["relative"].toBool(), true);
    auto* inactive = h.Send("/v1/scenarios/inactive/run", "POST");
    QTRY_VERIFY(inactive->isFinished());
    QCOMPARE(HttpStatus(inactive), 409);
    auto* run = h.Send("/v1/scenarios/evening/run", "POST");
    QTRY_VERIFY(run->isFinished());
    QCOMPARE(HttpStatus(run), 200);
    QCOMPARE(h.api.executions, QStringList{"evening"});
  }

  void RouteParametersMethodsAndBodyLimit() {
    Harness h;
    auto* encoded = h.Send("/v1/devices/room%2Flamp%20one");
    QTRY_VERIFY(encoded->isFinished());
    QCOMPARE(HttpStatus(encoded), 200);
    QCOMPARE(Body(encoded)["device"].toObject()["id"].toString(), QString("room/lamp one"));

    auto* trailing = h.Send("/v1/status/");
    QTRY_VERIFY(trailing->isFinished());
    QCOMPARE(HttpStatus(trailing), 200);
    for (const auto& path : {"/v1/devices/lamp/actions", "/v1/scenarios/evening/run"}) {
      auto* reply = h.Send(path);
      QTRY_VERIFY(reply->isFinished());
      QCOMPARE(HttpStatus(reply), 405);
    }
    auto* extra_segment = h.Send("/v1/devices/lamp/actions/extra", "POST");
    QTRY_VERIFY(extra_segment->isFinished());
    QCOMPARE(HttpStatus(extra_segment), 404);

    const QByteArray at_limit(RestProtocol::MaxBodyBytes, ' ');
    auto* accepted = h.Send("/v1/status", "GET", at_limit);
    QTRY_VERIFY(accepted->isFinished());
    QCOMPARE(HttpStatus(accepted), 200);
    auto* rejected = h.Send("/v1/status", "GET", at_limit + ' ');
    QTRY_VERIFY(rejected->isFinished());
    QCOMPARE(HttpStatus(rejected), 413);
    QCOMPARE(Body(rejected)["error"].toObject()["code"].toString(), QString("body_too_large"));
    auto* unauthorized = h.Send("/v1/status", "GET", at_limit + ' ', "example.invalid");
    QTRY_VERIFY(unauthorized->isFinished());
    QCOMPARE(HttpStatus(unauthorized), 403);
    QVERIFY(h.api.actions.isEmpty());
  }

  void StructuredErrors() {
    Harness h;
    for (const auto& [error, expected] :
      QList<QPair<ApiError, int>>{{{ApiErrorKind::Http, "missing", 404}, 404},
        {{ApiErrorKind::Http, "session expired", 401}, 401},
        {{ApiErrorKind::Timeout, "timeout"}, 504},
        {{ApiErrorKind::InvalidResponse, "bad response"}, 502}}) {
      h.api.error = error;
      auto* reply = h.Send("/v1/devices/lamp");
      QTRY_VERIFY(reply->isFinished());
      QCOMPARE(HttpStatus(reply), expected);
      QVERIFY(!Body(reply)["error"].toObject()["code"].toString().isEmpty());
    }
  }

  void OverlapTimeoutAndDisableCancelDelivery() {
    Harness h;
    const QPointer<QObject> unrelated_child(new QObject(&h.server));
    h.api.defer = true;
    auto* first = h.Send("/v1/devices/first");
    auto* second = h.Send("/v1/devices/second");
    QTRY_COMPARE(h.api.reads.size(), 2);
    h.api.reads[1].handler(ScriptedApi::Device(h.api.reads[1].id));
    h.api.reads[0].handler(ScriptedApi::Device(h.api.reads[0].id));
    QTRY_VERIFY(first->isFinished());
    QTRY_VERIFY(second->isFinished());
    QCOMPARE(Body(first)["device"].toObject()["id"].toString(), QString("first"));
    QCOMPARE(Body(second)["device"].toObject()["id"].toString(), QString("second"));
    auto* timed_out = h.Send("/v1/devices/timeout");
    QTRY_COMPARE(h.api.reads.size(), 3);
    QTRY_VERIFY(timed_out->isFinished());
    QCOMPARE(HttpStatus(timed_out), 504);
    QTRY_VERIFY(h.api.reads.last().context.isNull());
    auto* pending = h.Send("/v1/devices/pending");
    QTRY_COMPARE(h.api.reads.size(), 4);
    h.server.Stop();
    QVERIFY(h.api.reads.last().context.isNull());
    QVERIFY(unrelated_child);
    Q_UNUSED(pending);
    QVERIFY(h.server.Start(0));
  }

  void ControllerAndViewModelReflectExternalChanges() {
    QTemporaryDir directory;
    Settings settings(directory.filePath("settings.ini"));
    QLocalServer daemon;
    const auto name = "yh-rest-unit-" + QUuid::createUuid().toString(QUuid::Id128);
    QVERIFY(daemon.listen(name));
    bool running = true;
    int disables = 0;
    connect(&daemon, &QLocalServer::newConnection, this, [&] {
      auto* socket = daemon.nextPendingConnection();
      auto buffer = std::make_shared<QByteArray>();
      connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
      connect(socket, &QLocalSocket::readyRead, socket, [&, socket, buffer] {
        *buffer += socket->readAll();
        if (!buffer->contains('\n')) {
          return;
        }
        if (QJsonDocument::fromJson(*buffer).object()["command"] == "disable") {
          running = false;
          ++disables;
        }
        socket->write(
          QJsonDocument(QJsonObject{{"ok", true}, {"enabled", running}, {"running", running},
                          {"port", 9000}, {"url", "http://127.0.0.1:9000/v1"}})
            .toJson(QJsonDocument::Compact) +
          '\n');
        socket->disconnectFromServer();
      });
    });
    RestControlService service(&settings, {name, {}, {}, false});
    RestViewModel model(&service);
    QTRY_VERIFY(model.IsRunning());
    model.SetEnabled(true);
    QTRY_VERIFY(!model.IsBusy());
    QVERIFY(settings.GetRestEnabled());
    QCOMPARE(settings.GetRestPort(), quint16(9000));
    model.SetPort(9100);
    QCOMPARE(settings.GetRestPort(), quint16(9000));
    model.SetEnabled(false);
    QTRY_COMPARE(disables, 1);
    QTRY_VERIFY(!model.IsEnabled());
    QVERIFY(!settings.GetRestEnabled());
    model.SetPort(settings.GetMcpPort());
    QCOMPARE(settings.GetRestPort(), quint16(9000));
    QVERIFY(!model.GetError().isEmpty());
    model.SetPort(9100);
    QCOMPARE(settings.GetRestPort(), quint16(9100));
    QVERIFY(model.GetError().isEmpty());
    model.SetPort(0);
    QCOMPARE(settings.GetRestPort(), quint16(9100));
    QSignalSpy conflict(&service, &RestControlService::finished);
    service.Request(RestControlService::Command::Enable, settings.GetMcpPort());
    QCOMPARE(conflict.size(), 1);
    QCOMPARE(conflict.takeFirst()[0].toJsonObject()["error"].toObject()["code"].toString(), QString("port_conflict"));
    daemon.close();
    QSignalSpy results(&service, &RestControlService::finished);
    service.Request(RestControlService::Command::Status);
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(results.last()[0].toJsonObject()["ok"].toBool());
    QVERIFY(!results.last()[0].toJsonObject()["running"].toBool());
  }

  void DisableWaitsForWorkerExit_data() {
    QTest::addColumn<bool>("timeout");
    QTest::newRow("graceful-exit") << false;
    QTest::newRow("worker-does-not-exit") << true;
  }

  void DisableWaitsForWorkerExit() {
    QFETCH(bool, timeout);
    QTemporaryDir directory;
    const QDir files(directory.path());
    const auto name = "yh-rest-unit-" + QUuid::createUuid().toString(QUuid::Id128);
    QVERIFY(WriteFile(files.filePath("start")));
    QProcess worker;
    worker.start(QCoreApplication::applicationFilePath(),
                 {"--control-worker", name, directory.path()});
    QVERIFY(worker.waitForStarted(5000));
    WorkerCleanup cleanup{directory.path()};
    QTRY_VERIFY(QFileInfo::exists(files.filePath("ready")));
    Settings settings(files.filePath("settings.ini"));
    RestControlService service(&settings, {name, {}, {}, false});
    QSignalSpy results(&service, &RestControlService::finished);
    service.Request(RestControlService::Command::Disable, {}, timeout ? 500 : 5000);
    QTRY_VERIFY(QFileInfo::exists(files.filePath("disabled")));
    QVERIFY(service.IsBusy());
    QCOMPARE(results.size(), 0);
    QCOMPARE(worker.state(), QProcess::Running);
    if (timeout) {
      QTRY_COMPARE(results.size(), 1);
      const auto result = results.last()[0].toJsonObject();
      QVERIFY(!result["ok"].toBool());
      QCOMPARE(result["error"].toObject()["code"].toString(), QString("timeout"));
      QCOMPARE(worker.state(), QProcess::Running);
      QVERIFY(WriteFile(files.filePath("exit")));
      QVERIFY(worker.waitForFinished(5000));
    } else {
      QVERIFY(WriteFile(files.filePath("exit")));
      QTRY_COMPARE(results.size(), 1);
      QVERIFY(worker.waitForFinished(0) || worker.state() == QProcess::NotRunning);
      QVERIFY(results.last()[0].toJsonObject()["ok"].toBool());
      QVERIFY(!results.last()[0].toJsonObject()["running"].toBool());
    }
    QVERIFY(!service.IsBusy());
    QCOMPARE(worker.exitCode(), 0);
    QCOMPARE(worker.exitStatus(), QProcess::NormalExit);
  }

  void DisableSupersedesStartBeforeControlChannelExists() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    const auto name = "yh-rest-unit-" + QUuid::createUuid().toString(QUuid::Id128);
    WorkerCleanup cleanup{directory.path()};
    Settings settings(files.filePath("settings.ini"));
    RestControlService service(&settings, {name,
                                           QCoreApplication::applicationFilePath(),
                                           {"--control-worker", name, directory.path()},
                                           false});
    QSignalSpy results(&service, &RestControlService::finished);
    service.Request(RestControlService::Command::Enable, {}, 5000);
    QTRY_VERIFY(QFileInfo::exists(files.filePath("pid")));
    QVERIFY(service.IsBusy());
    service.Request(RestControlService::Command::Disable, {}, 5000);
    QVERIFY(WriteFile(files.filePath("start")));
    QTRY_VERIFY(QFileInfo::exists(files.filePath("disabled")));
    QVERIFY(service.IsBusy());
    QCOMPARE(results.size(), 0);
    QVERIFY(WriteFile(files.filePath("exit")));
    QTRY_COMPARE(results.size(), 1);
    const auto result = results.last()[0].toJsonObject();
    QVERIFY(result["ok"].toBool());
    QVERIFY(!result["running"].toBool());
    QVERIFY(!settings.GetRestEnabled());
    QVERIFY(!service.IsBusy());
  }
};

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  const auto arguments = app.arguments();
  if (arguments.value(1) == "--control-worker") {
    return RunControlWorker(app, arguments.value(2), arguments.value(3));
  }
  RestTests tests;
  return QTest::qExec(&tests, argc, argv);
}
#include "RestTests.moc"
