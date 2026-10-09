#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <memory>
#include "auth/AuthorizationFactory.h"
#include "auth/OAuthConfiguration.h"
#include "auth/QtOAuthAuthorizationFlow.h"

class OAuthAdapterTests final : public QObject {
  Q_OBJECT
private:
  static QJsonObject Configuration() {
    return {{"auth_url", "https://example.invalid/authorize"},
            {"access_token_url", "https://example.invalid/token"},
            {"client_id", "synthetic-client"}, {"client_secret", ""},
            {"redirect_base", "http://127.0.0.1/callback"}, {"redirect_port", 4321},
            {"scopes", QJsonArray{"iot:view", "iot:control", "iot:view"}}};
  }
  static AuthResult<OAuthConfiguration> Parse(const QJsonObject& object) {
    return OAuthConfiguration::Parse(QJsonDocument(object).toJson());
  }
private slots:
  void ParsesConfiguredRedirectAndScopes() {
    const auto config = Parse(Configuration());
    QVERIFY(config);
    QCOMPARE(config->redirect_port, quint16(4321));
    QCOMPARE(config->redirect_base.path(), QString("/callback"));
    QCOMPARE(config->scopes, (QSet<QByteArray>{"iot:view", "iot:control"}));
    QVERIFY(config->client_secret.isEmpty());
  }

  void AcceptsLocalhostRedirects_data() {
    QTest::addColumn<QString>("redirect_base");
    QTest::newRow("localhost-root") << QString("http://localhost");
    QTest::newRow("localhost-callback") << QString("http://localhost/callback");
  }
  void AcceptsLocalhostRedirects() {
    QFETCH(QString, redirect_base);
    auto object = Configuration();
    object["redirect_base"] = redirect_base;
    const auto config = Parse(object);
    QVERIFY(config);
    QCOMPARE(config->redirect_base, QUrl(redirect_base));
    QCOMPARE(config->redirect_port, quint16(4321));
  }

  void RejectsMalformedConfiguration_data() {
    QTest::addColumn<QString>("field");
    QTest::addColumn<QJsonValue>("value");
    QTest::newRow("blank-client") << QString("client_id") << QJsonValue("");
    QTest::newRow("wrong-client-type") << QString("client_id") << QJsonValue(123);
    QTest::newRow("invalid-auth-url") << QString("auth_url") << QJsonValue("garbage");
    QTest::newRow("insecure-auth-url") << QString("auth_url") << QJsonValue("http://example.invalid/authorize");
    QTest::newRow("insecure-token-url") << QString("access_token_url") << QJsonValue("http://example.invalid/token");
    QTest::newRow("nonlocal-callback") << QString("redirect_base") << QJsonValue("http://example.invalid");
    QTest::newRow("callback-query") << QString("redirect_base") << QJsonValue("http://127.0.0.1?unexpected=1");
    QTest::newRow("callback-port-in-base") << QString("redirect_base") << QJsonValue("http://127.0.0.1:1337");
    QTest::newRow("zero-port") << QString("redirect_port") << QJsonValue(0);
    QTest::newRow("large-port") << QString("redirect_port") << QJsonValue(65536);
    QTest::newRow("fractional-port") << QString("redirect_port") << QJsonValue(1337.5);
    QTest::newRow("string-port") << QString("redirect_port") << QJsonValue("1337");
    QTest::newRow("bad-scopes") << QString("scopes") << QJsonValue(QJsonArray{42});
    QTest::newRow("spaced-scope") << QString("scopes") << QJsonValue(QJsonArray{"two scopes"});
    QTest::newRow("nonarray-scopes") << QString("scopes") << QJsonValue("iot:view");
    QTest::newRow("wrong-secret-type") << QString("client_secret") << QJsonValue(123);
  }
  void RejectsMalformedConfiguration() {
    QFETCH(QString, field);
    QFETCH(QJsonValue, value);
    auto object = Configuration();
    object[field] = value;
    const auto result = Parse(object);
    QVERIFY(!result);
    QCOMPARE(result.error().kind, AuthErrorKind::Initialization);
  }

  void RejectsMissingFieldsAndMalformedJson() {
    for (const auto* field : {"auth_url", "access_token_url", "client_id", "client_secret",
                             "redirect_base", "redirect_port", "scopes"}) {
      auto object = Configuration();
      object.remove(field);
      QVERIFY(!Parse(object));
    }
    QVERIFY(!OAuthConfiguration::Parse("[]"));
    QVERIFY(!OAuthConfiguration::Parse("{"));
    QVERIFY(!OAuthConfiguration::Parse("null"));
  }

  void ConfigurationFailureIsLazyAndAsynchronous() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    int opened = 0;
    int completed = 0;
    QtOAuthAuthorizationFlow flow(nullptr, dir.filePath("absent.json"), dir.filePath("absent.html"),
      [&opened](const QUrl&) { ++opened; return true; });
    QCOMPARE(completed, 0);
    QObject context;
    flow.Start(&context, [&completed](AuthResult<QString> result) {
      QVERIFY(!result);
      QCOMPARE(result.error().kind, AuthErrorKind::Initialization);
      ++completed;
    });
    QCOMPARE(completed, 0);
    QTRY_COMPARE(completed, 1);
    QCOMPARE(opened, 0);
  }

  void CancellationAndContextDestructionSuppressSetupResults() {
    QtOAuthAuthorizationFlow flow(nullptr, ":/missing-config", ":/missing-callback",
      [](const QUrl&) { return false; });
    QObject context;
    int completed = 0;
    flow.Start(&context, [&completed](auto) { ++completed; });
    flow.Cancel();
    auto transient = std::make_unique<QObject>();
    flow.Start(transient.get(), [&completed](auto) { ++completed; });
    transient.reset();
    flow.Start(&context, [&completed](auto result) { QVERIFY(!result); ++completed; });
    QTRY_COMPARE(completed, 1);
  }

  void CompositionDoesNotInitializeBrowserOrReadKeychain() {
    std::unique_ptr<IAuthorizationService> saved(CreateAuthorizationService(AuthorizationMode::SavedTokenOnly, nullptr));
    std::unique_ptr<IAuthorizationService> interactive(CreateAuthorizationService(AuthorizationMode::Interactive, nullptr));
    QVERIFY(!saved->IsAuthorized());
    QVERIFY(!interactive->IsAuthorized());
    int saved_flows = 0;
    for (auto* child : saved->children()) {
      if (dynamic_cast<QtOAuthAuthorizationFlow*>(child)) { ++saved_flows; }
    }
    QCOMPARE(saved_flows, 0);
    // No credential resource is linked to this test. The failure must be visible
    // to observers of the operation, rather than emitted in a constructor.
    QSignalSpy failed(interactive.get(), &IAuthorizationService::initializationFailed);
    QCOMPARE(failed.size(), 0);
    interactive->AttemptAuthorization();
    QTRY_COMPARE(failed.size(), 1);
  }
};

QTEST_GUILESS_MAIN(OAuthAdapterTests)
#include "OAuthAdapterTests.moc"
