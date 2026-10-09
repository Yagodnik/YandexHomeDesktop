#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <memory>
#include "auth/AuthorizationService.h"
#include "models/AuthorizationModel.h"

namespace {
template<typename T>
struct Pending {
  QPointer<QObject> context;
  AuthResultHandler<T> handler;
  void Send(AuthResult<T> result) const { if (context) { handler(std::move(result)); } }
};

class ScriptedStore final : public ITokenStore {
public:
  QList<Pending<QString>> reads;
  QList<Pending<void>> writes;
  QList<Pending<void>> deletes;
  QStringList mutations;
  void Read(QObject* context, AuthResultHandler<QString> handler) override {
    reads.append({context, std::move(handler)});
  }
  void Write(const QString&, QObject* context, AuthResultHandler<void> handler) override {
    mutations.append("write");
    writes.append({context, std::move(handler)});
  }
  void Delete(QObject* context, AuthResultHandler<void> handler) override {
    mutations.append("delete");
    deletes.append({context, std::move(handler)});
  }
};

class ScriptedFlow final : public IAuthorizationFlow {
public:
  QList<Pending<QString>> attempts;
  int canceled = 0;
  void Start(QObject* context, AuthResultHandler<QString> handler) override {
    attempts.append({context, std::move(handler)});
  }
  void Cancel() override { ++canceled; }
};

class ImmediateStore final : public ITokenStore {
public:
  int deletes = 0;
  void Read(QObject*, AuthResultHandler<QString> handler) override { handler(QString{}); }
  void Write(const QString&, QObject*, AuthResultHandler<void> handler) override { handler({}); }
  void Delete(QObject*, AuthResultHandler<void> handler) override { ++deletes; handler({}); }
};
class ImmediateFlow final : public IAuthorizationFlow {
public:
  void Start(QObject*, AuthResultHandler<QString> handler) override { handler(QString("synthetic")); }
  void Cancel() override {}
};
}

class AuthorizationTests final : public QObject {
  Q_OBJECT
private slots:
  void SavedSessionUsesOnlyStorage() {
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy authorized(&auth, &IAuthorizationService::authorized);
    QVERIFY(!auth.IsAuthorized());
    QVERIFY(!auth.GetToken());
    QCOMPARE(auth.GetLastErrorCode(), QString("0"));
    auth.AttemptLocalAuthorization();
    auth.AttemptLocalAuthorization();
    QCOMPARE(store.reads.size(), 1);
    QCOMPARE(authorized.size(), 0);
    QVERIFY(flow.attempts.isEmpty());
    store.reads.first().Send(QString("synthetic"));
    QCOMPARE(authorized.size(), 1);
    QVERIFY(auth.IsAuthorized());
    QVERIFY(auth.GetToken().has_value());
    QVERIFY(store.writes.isEmpty());
  }

  void ReadErrors_data() {
    QTest::addColumn<AuthErrorKind>("kind");
    QTest::addColumn<QString>("signal");
    QTest::newRow("missing") << AuthErrorKind::NotFound << QString("unauthorized()");
    QTest::newRow("user-denied") << AuthErrorKind::Canceled << QString("authorizationCanceled()");
    QTest::newRow("backend-error") << AuthErrorKind::Storage << QString("authorizationFailed()");
  }
  void ReadErrors() {
    QFETCH(AuthErrorKind, kind);
    QFETCH(QString, signal);
    ScriptedStore store;
    AuthorizationService auth(&store);
    QSignalSpy outcome(&auth, auth.metaObject()->method(auth.metaObject()->indexOfSignal(signal.toUtf8())));
    auth.AttemptLocalAuthorization();
    store.reads.first().Send(std::unexpected(AuthError{kind, 7, "synthetic failure"}));
    QCOMPARE(outcome.size(), 1);
    QVERIFY(!auth.IsAuthorized());
    QVERIFY(!auth.GetToken());
    QCOMPARE(auth.GetLastErrorCode(), QString("7"));
    auth.AttemptLocalAuthorization();
    store.reads.last().Send(QString("synthetic"));
    QVERIFY(auth.IsAuthorized());
    QCOMPARE(auth.GetLastErrorCode(), QString("0"));
  }

  void EmptyStoredTokenDoesNotCreateSession() {
    ScriptedStore store;
    AuthorizationService auth(&store);
    QSignalSpy unauthorized(&auth, &IAuthorizationService::unauthorized);
    auth.AttemptLocalAuthorization();
    store.reads.first().Send(QString{});
    QCOMPARE(unauthorized.size(), 1);
    QVERIFY(!auth.IsAuthorized());
  }

  void InteractiveLoginSupersedesPendingRead() {
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy authorized(&auth, &IAuthorizationService::authorized);
    QSignalSpy failed(&auth, &IAuthorizationService::authorizationFailed);
    auth.AttemptLocalAuthorization();
    auth.AttemptAuthorization();
    auth.AttemptAuthorization();
    QCOMPARE(flow.attempts.size(), 1);
    store.reads.first().Send(QString("old synthetic"));
    store.reads.first().Send(std::unexpected(AuthError{AuthErrorKind::Storage, 7, {}}));
    QCOMPARE(authorized.size(), 0);
    QCOMPARE(failed.size(), 0);
    flow.attempts.first().Send(QString("new synthetic"));
    QCOMPARE(authorized.size(), 1);
    QVERIFY(auth.IsAuthorized());
    QCOMPARE(store.writes.size(), 1);
    store.writes.first().Send({});
  }

  void FlowErrors_data() {
    QTest::addColumn<AuthErrorKind>("kind");
    QTest::addColumn<QString>("signal");
    QTest::newRow("setup") << AuthErrorKind::Initialization << QString("initializationFailed()");
    QTest::newRow("denied") << AuthErrorKind::Canceled << QString("authorizationCanceled()");
    QTest::newRow("network") << AuthErrorKind::Authorization << QString("authorizationFailed()");
  }
  void FlowErrors() {
    QFETCH(AuthErrorKind, kind);
    QFETCH(QString, signal);
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy outcome(&auth, auth.metaObject()->method(auth.metaObject()->indexOfSignal(signal.toUtf8())));
    auth.AttemptAuthorization();
    flow.attempts.first().Send(std::unexpected(AuthError{kind, 0x80000001u, {}}));
    QCOMPARE(outcome.size(), 1);
    QCOMPARE(auth.GetLastErrorCode(), QString("80000001"));
    QVERIFY(!auth.IsAuthorized());
    QVERIFY(store.writes.isEmpty());
    auth.AttemptAuthorization();
    QCOMPARE(flow.attempts.size(), 2);
  }

  void EmptyGrantFailsWithoutPersistence() {
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy failed(&auth, &IAuthorizationService::authorizationFailed);
    auth.AttemptAuthorization();
    flow.attempts.first().Send(QString{});
    QCOMPARE(failed.size(), 1);
    QVERIFY(!auth.IsAuthorized());
    QVERIFY(store.writes.isEmpty());
  }

  void LogoutRejectsPendingReadAndIsIdempotent() {
    ScriptedStore store;
    AuthorizationService auth(&store);
    QSignalSpy authorized(&auth, &IAuthorizationService::authorized);
    QSignalSpy logout(&auth, &IAuthorizationService::logout);
    QSignalSpy finished(&auth, &IAuthorizationService::logoutFinished);
    auth.AttemptLocalAuthorization();
    auth.Logout();
    auth.Logout();
    auth.AttemptLocalAuthorization();
    auth.AttemptAuthorization();
    QCOMPARE(logout.size(), 1);
    QCOMPARE(store.deletes.size(), 1);
    QCOMPARE(store.reads.size(), 1);
    store.reads.first().Send(QString("stale synthetic"));
    QVERIFY(!auth.IsAuthorized());
    QCOMPARE(authorized.size(), 0);
    store.deletes.first().Send(std::unexpected(AuthError{AuthErrorKind::NotFound, 1, {}}));
    QCOMPARE(finished.size(), 1);
    auth.AttemptLocalAuthorization();
    QCOMPARE(store.reads.size(), 2);
  }

  void LogoutCancelsFlowAndRejectsLateGrant() {
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy authorized(&auth, &IAuthorizationService::authorized);
    auth.AttemptAuthorization();
    auth.Logout();
    QCOMPARE(flow.canceled, 1);
    flow.attempts.first().Send(QString("stale synthetic"));
    QCOMPARE(authorized.size(), 0);
    QVERIFY(store.writes.isEmpty());
    store.deletes.first().Send({});
    auth.AttemptAuthorization();
    flow.attempts.last().Send(QString("new synthetic"));
    QCOMPARE(authorized.size(), 1);
  }

  void LogoutWaitsForPendingWrite_data() {
    QTest::addColumn<bool>("write_succeeds");
    QTest::newRow("success") << true;
    QTest::newRow("failure") << false;
  }
  void LogoutWaitsForPendingWrite() {
    QFETCH(bool, write_succeeds);
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy finished(&auth, &IAuthorizationService::logoutFinished);
    auth.AttemptAuthorization();
    flow.attempts.first().Send(QString("synthetic"));
    QVERIFY(auth.IsAuthorized());
    auth.Logout();
    QVERIFY(!auth.IsAuthorized());
    QVERIFY(!auth.GetToken());
    QVERIFY(store.deletes.isEmpty());
    QCOMPARE(finished.size(), 0);
    store.writes.first().Send(write_succeeds ? AuthResult<void>{}
      : AuthResult<void>{std::unexpected(AuthError{AuthErrorKind::Storage, 7, {}})});
    QCOMPARE(store.mutations, (QStringList{"write", "delete"}));
    store.deletes.first().Send({});
    QCOMPARE(finished.size(), 1);
    QVERIFY(!auth.IsAuthorized());
  }

  void FailedDeletionKeepsSessionClearedAndCanBeRetried() {
    ScriptedStore store;
    AuthorizationService auth(&store);
    QSignalSpy failed(&auth, &IAuthorizationService::logoutFailed);
    auth.AttemptLocalAuthorization();
    store.reads.first().Send(QString("synthetic"));
    auth.Logout();
    store.deletes.first().Send(std::unexpected(AuthError{AuthErrorKind::Storage, 2, "synthetic failure"}));
    QCOMPARE(failed.size(), 1);
    QVERIFY(!auth.GetToken());
    QCOMPARE(auth.GetLastErrorCode(), QString("2"));
    auth.Logout();
    QCOMPARE(store.deletes.size(), 2);
  }

  void ReentrantWriteCompletionDuringLogoutStartsOneDeletion() {
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    auth.AttemptAuthorization();
    flow.attempts.first().Send(QString("synthetic"));
    connect(&auth, &IAuthorizationService::logout, &auth, [&store] { store.writes.first().Send({}); });
    auth.Logout();
    QCOMPARE(store.deletes.size(), 1);
  }

  void FailedWriteDoesNotInvalidateGrantedSession() {
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy authorized(&auth, &IAuthorizationService::authorized);
    QSignalSpy failed(&auth, &IAuthorizationService::authorizationFailed);
    auth.AttemptAuthorization();
    flow.attempts.first().Send(QString("synthetic"));
    store.writes.first().Send(std::unexpected(AuthError{AuthErrorKind::Storage, 7, {}}));
    QCOMPARE(authorized.size(), 1);
    QCOMPARE(failed.size(), 0);
    QVERIFY(auth.IsAuthorized());
    QCOMPARE(auth.GetLastErrorCode(), QString("7"));
  }

  void SavedTokenOnlyServiceCannotStartBrowserLogin() {
    ScriptedStore store;
    AuthorizationService auth(&store);
    QSignalSpy failed(&auth, &IAuthorizationService::authorizationFailed);
    auth.AttemptAuthorization();
    QCOMPARE(failed.size(), 1);
    QVERIFY(store.reads.isEmpty());
    QVERIFY(store.writes.isEmpty());
  }

  void DestroyedContextSuppressesOutstandingResults() {
    ScriptedStore store;
    ScriptedFlow flow;
    auto auth = std::make_unique<AuthorizationService>(&store, &flow);
    auth->AttemptLocalAuthorization();
    auth->AttemptAuthorization();
    auth.reset();
    QVERIFY(!store.reads.first().context);
    QVERIFY(!flow.attempts.first().context);
    store.reads.first().Send(QString("synthetic"));
    flow.attempts.first().Send(QString("synthetic"));
    QVERIFY(store.writes.isEmpty());
  }

  void InlineCompletionsSupportLogoutFromAuthorizedSignal() {
    ImmediateStore store;
    ImmediateFlow flow;
    AuthorizationService auth(&store, &flow);
    QSignalSpy finished(&auth, &IAuthorizationService::logoutFinished);
    connect(&auth, &IAuthorizationService::authorized, &auth, &IAuthorizationService::Logout);
    auth.AttemptAuthorization();
    QCOMPARE(finished.size(), 1);
    QCOMPARE(store.deletes, 1);
    QVERIFY(!auth.IsAuthorized());
  }

  void ModelPreservesQmlMethodsAndSignalsWithoutCredentials() {
    ScriptedStore store;
    ScriptedFlow flow;
    AuthorizationService auth(&store, &flow);
    AuthorizationModel model(&auth);
    QVERIFY(model.metaObject()->indexOfMethod("GetToken()") == -1);
    QSignalSpy authorized(&model, &AuthorizationModel::authorized);
    QSignalSpy logout(&model, &AuthorizationModel::logout);
    QSignalSpy finished(&model, &AuthorizationModel::logoutFinished);
    QVERIFY(QMetaObject::invokeMethod(&model, "AttemptLocalAuthorization"));
    store.reads.first().Send(QString("synthetic"));
    QCOMPARE(authorized.size(), 1);
    QVERIFY(model.IsAuthorized());
    QCOMPARE(model.GetLastErrorCode(), QString("0"));
    QVERIFY(QMetaObject::invokeMethod(&model, "Logout"));
    QCOMPARE(logout.size(), 1);
    store.deletes.first().Send({});
    QCOMPARE(finished.size(), 1);
    QVERIFY(!model.IsAuthorized());
    QVERIFY(QMetaObject::invokeMethod(&model, "AttemptAuthorization"));
    QCOMPARE(flow.attempts.size(), 1);
    for (const auto* signal : {"unauthorized()", "authorizationFailed()", "initializationFailed()",
                               "authorizationCanceled()", "logoutFailed(QString)"}) {
      QSignalSpy forwarded(&model, model.metaObject()->method(model.metaObject()->indexOfSignal(signal)));
      if (QByteArray(signal) == "logoutFailed(QString)") {
        QVERIFY(QMetaObject::invokeMethod(&auth, "logoutFailed", Q_ARG(QString, QString("synthetic failure"))));
      } else {
        const QByteArray method = QByteArray(signal).split('(').first();
        QVERIFY(QMetaObject::invokeMethod(&auth, method.constData()));
      }
      QCOMPARE(forwarded.size(), 1);
    }
  }
};

QTEST_GUILESS_MAIN(AuthorizationTests)
#include "AuthorizationTests.moc"
