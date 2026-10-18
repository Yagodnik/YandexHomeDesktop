#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QTest>

namespace {
QString executable;
QString worker_executable;
bool fixture_available = false;

struct Result {
  int code;
  QJsonObject json;
  QByteArray output;
};

Result Run(const QStringList& arguments) {
  QProcess process;
  process.start(executable, arguments + QStringList{"--json", "--timeout", "10000"});
  if (!process.waitForStarted(5000) || !process.waitForFinished(15000)) {
    return {-1, {}};
  }
  const auto bytes =
    process.exitCode() == 0 ? process.readAllStandardOutput() : process.readAllStandardError();
  return {process.exitCode(), QJsonDocument::fromJson(bytes).object(), bytes};
}

quint16 FreePort() {
  QTcpServer server;
  return server.listen(QHostAddress::LocalHost, 0) ? server.serverPort() : 0;
}
} // namespace

class RestProcessTests final : public QObject {
  Q_OBJECT
private slots:

  void JsonConfigurationAndExplicitTokenAvoidSecretStorage() {
    QTemporaryDir directory;
    const auto config_path = directory.filePath("rest config with spaces.json");
    const auto port = FreePort();
    QVERIFY(port != 0);
    QJsonObject configuration{{"host", "127.0.0.1"}, {"port", port},
      {"oauth_token", "synthetic-test-token"}};
    const auto write_configuration = [&] {
      QFile file(config_path);
      return file.open(QIODevice::WriteOnly) &&
        file.write(QJsonDocument(configuration).toJson()) > 0;
    };
    QVERIFY(write_configuration());
    const QStringList base{"--rest-config", config_path};
    struct Cleanup {
      QStringList base;
      ~Cleanup() { Run(base + QStringList{"--disable-rest"}); }
    } cleanup{base};
    QProcess foreground;
    foreground.setWorkingDirectory(directory.path());
    foreground.start(worker_executable, base + QStringList{"--json"});
    QVERIFY(foreground.waitForStarted(5000));
    QTRY_VERIFY_WITH_TIMEOUT(
      Run(base + QStringList{"--status-rest"}).json["running"].toBool(), 5000);
    auto result = Run(base + QStringList{"--enable-rest"});
    QCOMPARE(result.code, 0);
    QCOMPARE(result.json["host"].toString(), QString("127.0.0.1"));
    QCOMPARE(result.json["port"].toInt(), port);
    QCOMPARE(result.json["pid"].toInteger(), foreground.processId());
    QVERIFY(!result.output.contains("synthetic-test-token"));
    QVERIFY(!result.json.contains("oauth_token"));
    QNetworkAccessManager network;
    auto* reply = network.get(QNetworkRequest(QUrl(result.json["url"].toString() + "/status")));
    QTRY_VERIFY_WITH_TIMEOUT(reply->isFinished(), 5000);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    reply = network.get(QNetworkRequest(QUrl(result.json["url"].toString() + "/openapi.json")));
    QTRY_VERIFY_WITH_TIMEOUT(reply->isFinished(), 5000);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    const auto specification = reply->readAll();
    const auto document = QJsonDocument::fromJson(specification).object();
    QCOMPARE(document["openapi"].toString(), QString("3.1.0"));
    QCOMPARE(document["servers"].toArray().first().toObject()["url"].toString(),
      result.json["url"].toString().chopped(3));
    QVERIFY(!document.contains("ok"));
    QVERIFY(!specification.contains("synthetic-test-token"));
    configuration["host"] = "::1";
    QVERIFY(write_configuration());
    result = Run(base + QStringList{"--enable-rest"});
    QCOMPARE(result.code, 1);
    QCOMPARE(result.json["error"].toObject()["code"].toString(), QString("host_conflict"));
    configuration["host"] = "127.0.0.1";
    QVERIFY(write_configuration());
    QCOMPARE(Run(base + QStringList{"--disable-rest"}).code, 0);
    QVERIFY(foreground.waitForFinished(0));
    QCOMPARE(foreground.exitCode(), 0);
    QVERIFY(!foreground.readAllStandardOutput().contains("synthetic-test-token"));
    QVERIFY(!foreground.readAllStandardError().contains("synthetic-test-token"));
    const auto override_port = FreePort();
    QVERIFY(override_port != 0);
    result = Run(base + QStringList{"--enable-rest", "--rest-port", QString::number(override_port)});
    QCOMPARE(result.code, 0);
    QCOMPARE(result.json["port"].toInt(), override_port);
    QCOMPARE(result.json["url"].toString(), QString("http://127.0.0.1:%1/v1").arg(override_port));
    const auto worker_pid = result.json["pid"].toInteger();
    // Status/disable use the profile path as identity even if the file is gone.
    QVERIFY(QFile::remove(config_path));
    result = Run(base + QStringList{"--status-rest"});
    QCOMPARE(result.code, 0);
    QCOMPARE(result.json["pid"].toInteger(), worker_pid);
    QCOMPARE(Run(base + QStringList{"--disable-rest"}).code, 0);
    QTcpServer released;
    QVERIFY(released.listen(QHostAddress::LocalHost, override_port));
    result = Run(base + QStringList{"--enable-rest"});
    QCOMPARE(result.code, 2);
    QCOMPARE(result.json["error"].toObject()["code"].toString(), QString("config_error"));
  }

  void EntryPointControlAndFixtureRest() {
    QCOMPARE(Run({"--status-rest", "--json", "-j"}).code, 2);
    QCOMPARE(Run({"--status-rest", "devices", "list"}).code, 2);
    if (!fixture_available) {
      const auto result = Run({"--fake-api", "--enable-rest"});
      QCOMPARE(result.code, 2);
      QVERIFY(!result.json["ok"].toBool());
      return;
    }
    QTemporaryDir directory;
    const auto fixture = directory.filePath("fixture with spaces.json");
    QVERIFY(QFile::copy(FIXTURE_PATH, fixture));
    const QStringList base{"--fake-api", "--fake-api-data", fixture};

    // RAII cleanup also runs if an assertion fails after a background process starts.
    struct Cleanup {
      QStringList base;

      ~Cleanup() {
        Run(base + QStringList{"--disable-rest"});
      }
    } cleanup{base};

    const auto port = FreePort();
    QVERIFY(port != 0);
    auto result = Run(base + QStringList{"--status-rest"});
    QCOMPARE(result.code, 0);
    QVERIFY(!result.json["running"].toBool());
    result = Run(base + QStringList{"--enable-rest", "--rest-port", QString::number(port)});
    QCOMPARE(result.code, 0);
    QVERIFY(result.json["running"].toBool());
    QCOMPARE(result.json["port"].toInt(), port);
    QVERIFY(!result.json.contains("token"));
    QCOMPARE(Run(base + QStringList{"--serve-rest"}).code, 2);
    const auto worker_pid = result.json["pid"].toInteger();
    QVERIFY(worker_pid > 0);
    result = Run(base + QStringList{"--enable-rest"}); // Idempotent, including a non-default port.
    QCOMPARE(result.code, 0);
    QCOMPARE(result.json["port"].toInt(), port);
    QCOMPARE(result.json["pid"].toInteger(), worker_pid);
    QNetworkAccessManager network;
    QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1/v1/devices").arg(port)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto* reply = network.get(request);
    QTRY_VERIFY_WITH_TIMEOUT(reply->isFinished(), 5000);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    const auto devices = QJsonDocument::fromJson(reply->readAll()).object()["devices"].toArray();
    QVERIFY(!devices.isEmpty());
    QCOMPARE(devices[0].toObject()["name"].toString(), QString("Desk lamp"));
    QProcess example;
    example.start(PYTHON_PATH, {EXAMPLE_PATH, "--cli", executable, "--fake-api", "--fake-api-data",
                                 fixture, "--device-id", "lamp", "--scenario-id", "evening"});
    QVERIFY(example.waitForStarted(5000));
    QVERIFY(example.waitForFinished(10000));
    QCOMPARE(example.exitCode(), 0);
    QVERIFY(example.readAllStandardOutput().contains("Desk lamp"));
    request.setUrl(QUrl(QString("http://127.0.0.1:%1/v1/scenarios/evening/run").arg(port)));
    reply = network.post(request, QByteArray{});
    QTRY_VERIFY_WITH_TIMEOUT(reply->isFinished(), 5000);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    const auto other_port = FreePort();
    QVERIFY(other_port != 0);
    result = Run(base + QStringList{"--enable-rest", "--rest-port", QString::number(other_port)});
    QCOMPARE(result.code, 1);
    QCOMPARE(result.json["error"].toObject()["code"].toString(), QString("port_conflict"));
    result = Run(base + QStringList{"--disable-rest"});
    QCOMPARE(result.code, 0);
    QVERIFY(!result.json["running"].toBool());
    QTcpServer released;
    QTRY_VERIFY_WITH_TIMEOUT(
      released.isListening() || released.listen(QHostAddress::LocalHost, port), 5000);
    released.close();
    result = Run(base + QStringList{"--status-rest"});
    QCOMPARE(result.code, 0);
    QVERIFY(!result.json["running"].toBool());
    result = Run(base + QStringList{"--disable-rest"}); // Repeated disable succeeds.
    QCOMPARE(result.code, 0);
    // Simultaneous enable commands converge on one worker and one listener.
    const auto start_arguments =
      base + QStringList{"--enable-rest", "--rest-port", QString::number(port), "--json"};
    QProcess first_start, second_start;
    first_start.start(executable, start_arguments);
    second_start.start(executable, start_arguments);
    QVERIFY(first_start.waitForStarted(5000));
    QVERIFY(second_start.waitForStarted(5000));
    QVERIFY(first_start.waitForFinished(10000));
    QVERIFY(second_start.waitForFinished(10000));
    QCOMPARE(first_start.exitCode(), 0);
    QCOMPARE(second_start.exitCode(), 0);
    result = Run(base + QStringList{"reset", "--i-know-what-i-am-doing"});
    QCOMPARE(result.code, 0);
    result = Run(base + QStringList{"--status-rest"});
    QVERIFY(!result.json["running"].toBool());
    // The foreground path also accepts disable and releases its ownership lock.
    QProcess foreground;
    foreground.start(worker_executable,
      base + QStringList{"--rest-port", QString::number(port), "--json"});
    QVERIFY(foreground.waitForStarted(5000));
    QTRY_VERIFY2_WITH_TIMEOUT(
      Run(base + QStringList{"--status-rest"}).json["running"].toBool(),
      foreground.readAllStandardError().constData(), 5000);
    QCOMPARE(Run(base + QStringList{"--status-rest"}).json["pid"].toInteger(), foreground.processId());
    QCOMPARE(Run(base + QStringList{"--disable-rest"}).code, 0);
    // Successful disable must mean the worker has exited, not just closed HTTP.
    QVERIFY(foreground.waitForFinished(0));
    QCOMPARE(foreground.exitCode(), 0);
    // Occupied ports fail without announcing an enabled listener.
    QVERIFY(released.listen(QHostAddress::LocalHost, port));
    result = Run(base + QStringList{"--enable-rest", "--rest-port", QString::number(port)});
    QCOMPARE(result.code, 1);
    QCOMPARE(result.json["error"].toObject()["code"].toString(), QString("listen_error"));
  }
};

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  if (argc < 4) {
    return 2;
  }
  executable = QString::fromLocal8Bit(argv[1]);
  worker_executable = QString::fromLocal8Bit(argv[2]);
  fixture_available = QString::fromLocal8Bit(argv[3]) == "Debug";
  // Keep the executable/configuration arguments out of Qt Test's parser while
  // forwarding discovery options and individual test selectors.
  QList<char*> test_argv{argv[0]};
  for (int index = 4; index < argc; ++index) {
    test_argv.append(argv[index]);
  }
  RestProcessTests tests;
  return QTest::qExec(&tests, test_argv.size(), test_argv.data());
}

#include "RestProcessTests.moc"
