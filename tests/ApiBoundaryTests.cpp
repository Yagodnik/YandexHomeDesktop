#include "ApiBoundaryTests.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QtTest>

#include "api/QtHttpTransport.h"
#include "api/YandexAccountApi.h"
#include "api/YandexHomeApi.h"

namespace {
class FakeTransport final : public IHttpTransport {
public:
  struct Pending {
    QByteArray method;
    QNetworkRequest request;
    QByteArray body;
    QPointer<QObject> context;
    ApiResultHandler<HttpResponse> handler;
  };

  QList<Pending> pending;

  void Get(const QNetworkRequest& request, QObject* context,
           ApiResultHandler<HttpResponse> handler) override {
    pending.append({"GET", request, {}, context, std::move(handler)});
  }

  void Post(const QNetworkRequest& request, const QByteArray& body, QObject* context,
            ApiResultHandler<HttpResponse> handler) override {
    pending.append({"POST", request, body, context, std::move(handler)});
  }

  void Reply(int index, ApiResult<HttpResponse> result) {
    if (pending[index].context) {
      pending[index].handler(std::move(result));
    }
  }
};

HttpResponse Json(const QByteArray& body) {
  return {body, 200};
}

class HangingReply final : public QNetworkReply {
public:
  HangingReply(const QNetworkRequest& request, QObject* parent) : QNetworkReply(parent) {
    setRequest(request);
    setUrl(request.url());
    setOperation(QNetworkAccessManager::GetOperation);
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    setFinished(false);
  }

  void abort() override {
    setError(QNetworkReply::OperationCanceledError, "aborted");
    setFinished(true);
    emit finished();
  }

protected:
  qint64 readData(char*, qint64) override { return -1; }
};

class HangingNetworkManager final : public QNetworkAccessManager {
public:
  using QNetworkAccessManager::QNetworkAccessManager;

protected:
  QNetworkReply* createRequest(Operation, const QNetworkRequest& request,
                               QIODevice*) override {
    return new HangingReply(request, this);
  }
};
}

void ApiBoundaryTests::HomeRequestsAreScoped() {
  FakeTransport transport;
  YandexHomeApi api([] { return QString("test-token"); }, &transport);
  QObject first_context;
  QObject second_context;
  QString first_id;
  QString second_id;

  api.GetUserInfo(&first_context, [&](ApiResult<UserInfo> result) {
    QVERIFY(result.has_value());
    first_id = result->request_id;
  });
  api.GetUserInfo(&second_context, [&](ApiResult<UserInfo> result) {
    QVERIFY(result.has_value());
    second_id = result->request_id;
  });

  QCOMPARE(transport.pending.size(), 2);
  QCOMPARE(transport.pending[0].method, QByteArray("GET"));
  QCOMPARE(transport.pending[0].request.rawHeader("Authorization"), QByteArray("Bearer test-token"));
  transport.Reply(1, Json(R"({"status":"ok","request_id":"second"})"));
  QCOMPARE(first_id, QString());
  QCOMPARE(second_id, QString("second"));
  transport.Reply(0, Json(R"({"status":"ok","request_id":"first"})"));
  QCOMPARE(first_id, QString("first"));
}

void ApiBoundaryTests::ParsingAndScenarioResults() {
  FakeTransport transport;
  YandexHomeApi api([] { return QString("token"); }, &transport);
  QObject context;
  QList<ApiErrorKind> errors;

  api.GetUserInfo(&context, [&](ApiResult<UserInfo> result) {
    QVERIFY(!result.has_value());
    errors.append(result.error().kind);
  });
  transport.Reply(0, Json("[]"));

  api.GetDeviceInfo("device-1", &context, [&](ApiResult<DeviceInfo> result) {
    QVERIFY(!result.has_value());
    errors.append(result.error().kind);
  });
  QCOMPARE(transport.pending[1].request.url().path(), QString("/v1.0/devices/device-1"));
  transport.Reply(1, Json(R"({"status":"error","message":"denied"})"));
  QCOMPARE(errors, (QList<ApiErrorKind>{ApiErrorKind::InvalidResponse, ApiErrorKind::Service}));

  api.GetUserInfo(&context, [&](ApiResult<UserInfo> result) {
    QVERIFY(!result.has_value());
    errors.append(result.error().kind);
  });
  transport.Reply(2, std::unexpected(ApiError{ApiErrorKind::Network, "offline"}));
  QCOMPARE(errors.last(), ApiErrorKind::Network);

  QList<ScenarioObject> scenarios;
  api.GetScenarios(&context, [&](ApiResult<QList<ScenarioObject>> result) {
    QVERIFY(result.has_value());
    scenarios = *result;
  });
  transport.Reply(3, Json(R"({"status":"ok","scenarios":[{"id":"s1","name":"Evening","is_active":true}]})"));
  QCOMPARE(scenarios.size(), 1);
  QCOMPARE(scenarios[0].id, QString("s1"));

  int executions = 0;
  api.ExecuteScenario("s1", &context, [&](ApiResult<void> result) {
    QVERIFY(result.has_value());
    ++executions;
  });
  QCOMPARE(transport.pending[4].method, QByteArray("POST"));
  QCOMPARE(transport.pending[4].request.url().path(), QString("/v1.0/scenarios/s1/actions"));
  transport.Reply(4, Json(R"({"status":"ok"})"));
  QCOMPARE(executions, 1);
}

void ApiBoundaryTests::ActionEventsArePreserved() {
  FakeTransport transport;
  YandexHomeApi api([] { return QString("token"); }, &transport);
  QObject context;
  QList<ApiResult<void>> events;

  CapabilityObject action;
  action.type = CapabilityType::OnOff;
  action.state = {{"instance", "on"}, {"value", true}};
  DeviceActionsObject device;
  device.id = "device-1";
  device.actions = {action};
  api.PerformActions({device}, &context, [&](ApiResult<void> result) {
    events.append(std::move(result));
  });

  QCOMPARE(transport.pending.size(), 1);
  QCOMPARE(transport.pending[0].method, QByteArray("POST"));
  QVERIFY(transport.pending[0].body.contains("device-1"));
  transport.Reply(0, Json(R"({"status":"ok","devices":[{"id":"device-1","capabilities":[{"state":{"action_result":{"status":"DONE"}}},{"state":{"action_result":{"status":"ERROR","error_code":"BAD"}}}]}]})"));
  QCOMPARE(events.size(), 2);
  QVERIFY(events[0].has_value());
  QVERIFY(!events[1].has_value());
  QCOMPARE(events[1].error().message, QString("BAD"));
}

void ApiBoundaryTests::AccountResults() {
  FakeTransport transport;
  YandexAccountApi api([] { return QString("token"); }, &transport);
  QObject context;
  AccountInfo account;
  api.LoadData(&context, [&](ApiResult<AccountInfo> result) {
    QVERIFY(result.has_value());
    account = *result;
  });
  QCOMPARE(transport.pending[0].request.url().host(), QString("login.yandex.ru"));
  transport.Reply(0, Json(R"({"display_name":"Ada","default_avatar_id":"avatar"})"));
  QCOMPARE(account.display_name, QString("Ada"));
  QCOMPARE(account.default_avatar_id, QString("avatar"));
}

void ApiBoundaryTests::TimeoutReportsBothLegacyErrors() {
  HangingNetworkManager manager;
  QtHttpTransport transport(nullptr, 10, &manager);
  QObject context;
  QList<ApiErrorKind> errors;
  transport.Get(QNetworkRequest(QUrl("https://example.invalid/hang")), &context,
                [&](ApiResult<HttpResponse> result) {
    QVERIFY(!result.has_value());
    errors.append(result.error().kind);
  });

  QTRY_COMPARE_WITH_TIMEOUT(errors.size(), 2, 1000);
  QVERIFY(errors.contains(ApiErrorKind::Network));
  QVERIFY(errors.contains(ApiErrorKind::Timeout));
}

void ApiBoundaryTests::DestroyedContextSuppressesEvents() {
  HangingNetworkManager manager;
  QtHttpTransport transport(nullptr, 10, &manager);
  auto* context = new QObject;
  int delivered = 0;
  transport.Get(QNetworkRequest(QUrl("https://example.invalid/hang")), context,
                [&](ApiResult<HttpResponse>) { ++delivered; });
  delete context;
  QTest::qWait(50);
  QCOMPARE(delivered, 0);
}
