#include "rest/RestServer.h"
#include "rest/RestEndpoints.h"
#include "services/HomeService.h"
#include "services/DeviceService.h"
#include "services/ScenarioService.h"
#include "services/AccountService.h"
#include "models/RestViewModel.h"
#include "app/common/StartupOptions.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocalServer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

namespace {
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
      handler(Device(id));
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
    model.SetEnabled(false);
    QTRY_COMPARE(disables, 1);
    QTRY_VERIFY(!model.IsEnabled());
    QVERIFY(!settings.GetRestEnabled());
    daemon.close();
    QSignalSpy results(&service, &RestControlService::finished);
    service.Request(RestControlService::Command::Status);
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(results.last()[0].toJsonObject()["ok"].toBool());
    QVERIFY(!results.last()[0].toJsonObject()["running"].toBool());
  }
};

QTEST_GUILESS_MAIN(RestTests)
#include "RestTests.moc"
