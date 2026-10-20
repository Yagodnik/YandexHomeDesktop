#include "worker/WorkerControlClient.h"
#include "worker/WorkerControlServer.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QUuid>
#include <memory>
#include <vector>

namespace {
using Command = WorkerControlClient::Command;

bool Write(const QString& path, const QByteArray& bytes = "ready") {
  QFile file(path);
  return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

QString Endpoint(const QTemporaryDir& directory) {
#ifdef Q_OS_UNIX
  // Keep Unix socket paths below the platform's length limit.
  return directory.filePath("control");
#else
  return "yh-worker-test-" + QUuid::createUuid().toString(QUuid::Id128);
#endif
}

WorkerLaunchConfiguration Configuration(const QString& name, const QString& directory,
                                        const QString& behavior = "normal") {
  return {
      name, QCoreApplication::applicationFilePath(), {"--worker-test", name, directory, behavior}};
}

QJsonObject Result(const QSignalSpy& results, qsizetype index = -1) {
  return results.at(index < 0 ? results.size() - 1 : index).at(0).toJsonObject();
}

QString Code(const QSignalSpy& results) {
  return Result(results)["error"].toObject()["code"].toString();
}

struct Cleanup {
  QString directory;
  ~Cleanup() {
    Write(QDir(directory).filePath("exit"));
    QFile endpoint(QDir(directory).filePath("endpoint"));
    if (!endpoint.open(QIODevice::ReadOnly))
      return;
    WorkerControlClient client({QString::fromUtf8(endpoint.readAll()), {}, {}});
    QEventLoop loop;
    QObject::connect(&client, &WorkerControlClient::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(0, &loop, [&] { client.Request(Command::Stop, {}, {}, 2000); });
    loop.exec();
  }
};

int RunWorker(QCoreApplication& app, const QString& name, const QString& directory,
              const QString& behavior) {
  const QDir files(directory);
  if (!Write(files.filePath("endpoint"), name.toUtf8()))
    return 2;
  if (!Write(files.filePath("spawned-" + QString::number(app.applicationPid()))))
    return 2;
  WorkerControlServer server;
  bool acquired = false;
  bool listening = false;
  bool stopped = false;
  bool ready = behavior != "not-ready";
  int stop_count = 0;
  server.SetHandler([&](const QJsonObject& request) -> QJsonObject {
    const auto command = request["command"].toString();
    if (command == "stop") {
      stopped = true;
      ready = false;
      Write(files.filePath("stopped"), QByteArray::number(++stop_count));
      if (behavior == "close-before-exit") {
        QTimer::singleShot(0, &server, &WorkerControlServer::Close);
      } else if (behavior != "hold-exit" && behavior != "gated" && behavior != "before-acquire") {
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
      }
    } else if (command == "echo") {
      return {{"ok", true}, {"value", request["value"]}};
    }
    return {{"ok", true}, {"running", ready && !stopped}, {"starting", !ready && !stopped}};
  });
  QTimer gates;
  QObject::connect(&gates, &QTimer::timeout, &app, [&] {
    if (QFile::exists(files.filePath("exit"))) {
      app.quit();
      return;
    }
    if (!acquired) {
      if (behavior == "before-acquire" && !QFile::exists(files.filePath("acquire")))
        return;
      if (!server.Acquire(name)) {
        Write(files.filePath("duplicate-" + QString::number(app.applicationPid())));
        app.exit(17);
        return;
      }
      acquired = true;
      Write(files.filePath("owned"));
    }
    if (behavior == "exit-before-listen") {
      app.exit(3);
      return;
    }
    if (!listening) {
      if ((behavior == "gated" || behavior == "before-acquire") &&
          !QFile::exists(files.filePath("listen")))
        return;
      if (!server.Listen()) {
        app.exit(4);
        return;
      }
      listening = true;
      Write(files.filePath("listening"));
    }
    if (QFile::exists(files.filePath("ready")))
      ready = true;
  });
  gates.start(5);
  QTimer::singleShot(20'000, &app, &QCoreApplication::quit);
  return app.exec();
}

int RunClient(QCoreApplication& app, const QStringList& args) {
  WorkerControlClient client(Configuration(args.value(3), args.value(4), args.value(5)));
  const auto command = args.value(2) == "start"  ? Command::Start
                       : args.value(2) == "stop" ? Command::Stop
                                                 : Command::Status;
  QObject::connect(&client, &WorkerControlClient::finished, &app, [&](const QJsonObject& result) {
    QFile output;
    if (!output.open(stdout, QIODevice::WriteOnly)) {
      app.exit(2);
      return;
    }
    output.write(QJsonDocument(result).toJson(QJsonDocument::Compact));
    output.flush();
    app.exit(result["ok"].toBool() ? 0 : 1);
  });
  QTimer::singleShot(0, &app,
                     [&] { client.Request(command, {}, {}, args.value(6, "5000").toInt()); });
  return app.exec();
}

int RunProbes(QCoreApplication& app, const QString& name, const QString& directory) {
  WorkerControlClient client({name, {}, {}});
  const QDir files(directory);
  QObject::connect(&client, &WorkerControlClient::finished, &app, [&](const QJsonObject&) {
    Write(files.filePath("probe-" + QString::number(app.applicationPid())));
    if (QFile::exists(files.filePath("stop-probes")))
      app.quit();
    else
      QTimer::singleShot(0, &client, [&] { client.Request(Command::Status); });
  });
  QTimer::singleShot(0, &client, [&] { client.Request(Command::Status); });
  QTimer::singleShot(15'000, &app, &QCoreApplication::quit);
  return app.exec();
}

// A raw peer lets tests deliberately send invalid or fragmented protocol data.
struct Peer {
  QLocalServer server;
  std::function<void(QLocalSocket*, const QByteArray&)> handler;
  int requests = 0;
  explicit Peer(const QString& name) {
    QObject::connect(&server, &QLocalServer::newConnection, &server, [this] {
      while (auto* socket = server.nextPendingConnection()) {
        auto bytes = std::make_shared<QByteArray>();
        auto handled = std::make_shared<bool>(false);
        QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        QObject::connect(socket, &QLocalSocket::readyRead, socket, [this, socket, bytes, handled] {
          *bytes += socket->readAll();
          if (!*handled && bytes->contains('\n')) {
            *handled = true;
            ++requests;
            if (handler)
              handler(socket, *bytes);
          }
        });
      }
    });
    if (!server.listen(name))
      qFatal("Cannot start worker test peer");
  }
};
} // namespace

class WorkerControlTests final : public QObject {
  Q_OBJECT
private slots:
  void MissingWorkerStatusAndStopAreIdempotent() {
    QTemporaryDir directory;
    WorkerControlClient client(Configuration(Endpoint(directory), directory.path()));
    QSignalSpy results(&client, &WorkerControlClient::finished);
    for (const auto command : {Command::Status, Command::Stop, Command::Stop}) {
      const auto count = results.size();
      client.Request(command);
      QTRY_COMPARE(results.size(), count + 1);
      QVERIFY(Result(results)["ok"].toBool());
      QVERIFY(!Result(results)["running"].toBool());
      QVERIFY(!client.IsBusy());
    }
    QCOMPARE(QDir(directory.path()).entryList({"spawned-*"}, QDir::Files).size(), 0);
  }

  void LaunchFailureCompletesOnceAndCanRetry() {
    QTemporaryDir directory;
    auto configuration = Configuration(Endpoint(directory), directory.path());
    configuration.executable = directory.filePath("missing executable");
    WorkerControlClient client(configuration);
    QSignalSpy results(&client, &WorkerControlClient::finished);
    for (int count = 1; count <= 2; ++count) {
      client.Request(Command::Start, {}, {}, 1000);
      QTRY_COMPARE(results.size(), count);
      QCOMPARE(Code(results), QString("startup_error"));
      QVERIFY(!client.IsBusy());
    }
  }

  void EmptyEndpointAndUnwritableDirectoryAreRejected() {
    QTemporaryDir directory;
    WorkerControlClient empty({});
    QSignalSpy empty_results(&empty, &WorkerControlClient::finished);
    empty.Request(Command::Start);
    QCOMPARE(empty_results.size(), 1);
    QCOMPARE(Code(empty_results), QString("control_error"));
#ifdef Q_OS_UNIX
    QVERIFY(Write(directory.filePath("file")));
    WorkerControlClient blocked({directory.filePath("file/control"), {}, {}});
    QSignalSpy results(&blocked, &WorkerControlClient::finished);
    blocked.Request(Command::Start);
    QCOMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("control_error"));
#endif
  }

  void ServerOwnershipPreventsSocketReplacement() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    auto first = std::make_unique<WorkerControlServer>();
    first->SetHandler([](const QJsonObject&) {
      return QJsonObject{{"ok", true}, {"running", true}, {"owner", "first"}};
    });
    QVERIFY(first->Start(name));
    WorkerControlServer second;
    QVERIFY(!second.Start(name));
    QCOMPARE(second.ErrorCode(), QString("already_running"));
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Status);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Result(results)["owner"].toString(), QString("first"));
    first->Close();
    QVERIFY(!second.Acquire(name));
    first.reset();
    QVERIFY(second.Start(name));
  }

  void GenericMessagesPreservePayloadAndUseRealPid() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    WorkerControlServer server;
    server.SetHandler([](const QJsonObject& request) {
      return QJsonObject{{"ok", true}, {"value", request["value"]}, {"pid", 123}};
    });
    QVERIFY(server.Start(name));
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    const QJsonObject value{{"nested", QJsonObject{{"text", "room with spaces"}}}};
    client.Request(Command::Send, {{"command", "reload"}, {"value", value}});
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Result(results)["value"].toObject(), value);
    QCOMPARE(Result(results)["pid"].toInteger(), QCoreApplication::applicationPid());
  }

  void InvalidResponses_data() {
    QTest::addColumn<QByteArray>("bytes");
    QTest::newRow("malformed") << QByteArray("{broken}\n");
    QTest::newRow("array") << QByteArray("[]\n");
    QTest::newRow("missing-ok") << QByteArray("{\"running\":true}\n");
    QTest::newRow("wrong-ok-type") << QByteArray("{\"ok\":1,\"running\":true}\n");
    QTest::newRow("missing-running") << QByteArray("{\"ok\":true}\n");
    QTest::newRow("wrong-running-type") << QByteArray("{\"ok\":true,\"running\":\"true\"}\n");
    QTest::newRow("negative-pid") << QByteArray("{\"ok\":true,\"running\":true,\"pid\":-1}\n");
    QTest::newRow("fractional-pid") << QByteArray("{\"ok\":true,\"running\":true,\"pid\":1.5}\n");
    QTest::newRow("string-pid") << QByteArray("{\"ok\":true,\"running\":true,\"pid\":\"1\"}\n");
    QTest::newRow("oversized") << QByteArray(65537, 'a');
  }

  void InvalidResponses() {
    QFETCH(QByteArray, bytes);
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    Peer peer(name);
    peer.handler = [bytes](QLocalSocket* socket, const QByteArray&) { socket->write(bytes); };
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Status, {}, {}, 2000);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("invalid_response"));
  }

  void FragmentedResponseCompletesOnlyAfterNewline() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    Peer peer(name);
    QPointer<QLocalSocket> pending;
    peer.handler = [&](QLocalSocket* socket, const QByteArray&) {
      pending = socket;
      socket->write("{\"ok\":true,");
    };
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Status);
    QTRY_VERIFY(pending);
    QCOMPARE(results.size(), 0);
    pending->write("\"running\":true}");
    QCoreApplication::processEvents();
    QCOMPARE(results.size(), 0);
    pending->write("\n");
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["running"].toBool());
  }

  void SilentPeerTimesOutAndNextRequestWorks() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    Peer peer(name);
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Status, {}, {}, 100);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("timeout"));
    peer.handler = [](QLocalSocket* socket, const QByteArray&) {
      socket->write("{\"ok\":true,\"running\":true}\n");
    };
    client.Request(Command::Status);
    QTRY_COMPARE(results.size(), 2);
    QVERIFY(Result(results)["ok"].toBool());
  }

  void GenericMutationIsNotRetriedAfterDisconnect_data() {
    QTest::addColumn<bool>("report_socket_errors");
    QTest::newRow("socket-error") << true;
    QTest::newRow("disconnect-only") << false;
  }

  void GenericMutationIsNotRetriedAfterDisconnect() {
    QFETCH(bool, report_socket_errors);
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    Peer peer(name);
    peer.handler = [](QLocalSocket* socket, const QByteArray&) { socket->abort(); };
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Send, {{"command", "mutate"}}, {}, 1000);
    if (!report_socket_errors) {
      // Exercise Windows' disconnect-only notification on every platform.
      auto* socket = client.findChild<QLocalSocket*>();
      QVERIFY(socket);
      QVERIFY(disconnect(socket, &QLocalSocket::errorOccurred, nullptr, nullptr));
    }
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("control_error"));
    QCOMPARE(peer.requests, 1);
  }

  void SupersededSocketCannotCompleteReplacement() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    Peer peer(name);
    QPointer<QLocalSocket> old_socket;
    peer.handler = [&](QLocalSocket* socket, const QByteArray& bytes) {
      if (QJsonDocument::fromJson(bytes).object()["command"] == "status") {
        old_socket = socket;
      } else {
        socket->write("{\"ok\":true,\"running\":false}\n");
      }
    };
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Status);
    QTRY_VERIFY(old_socket);
    client.Request(Command::Stop);
    if (old_socket)
      old_socket->write("{\"ok\":true,\"running\":true}\n");
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["ok"].toBool());
    QVERIFY(!Result(results)["running"].toBool());
  }

  void CompletionCanStartAnotherRequestOrDestroyClient() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    WorkerControlServer server;
    server.SetHandler(
        [](const QJsonObject&) { return QJsonObject{{"ok", true}, {"running", true}}; });
    QVERIFY(server.Start(name));
    auto* client = new WorkerControlClient({name, {}, {}});
    QPointer<WorkerControlClient> alive(client);
    int completions = 0;
    connect(client, &WorkerControlClient::finished, this, [&](const QJsonObject&) {
      if (++completions == 1)
        client->Request(Command::Status);
      else
        delete client;
    });
    client->Request(Command::Status);
    QTRY_COMPARE(completions, 2);
    QVERIFY(!alive);
  }

  void DestroyingClientCancelsPendingCallbacks() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    Peer peer(name);
    auto* client = new WorkerControlClient({name, {}, {}});
    QPointer<WorkerControlClient> alive(client);
    int completions = 0;
    connect(client, &WorkerControlClient::finished, this,
            [&](const QJsonObject&) { ++completions; });
    peer.handler = [&](QLocalSocket*, const QByteArray&) {
      delete client;
      client = nullptr;
    };
    client->Request(Command::Status);
    QTRY_COMPARE(peer.requests, 1);
    QVERIFY(!alive);
    // A fresh request supplies an event-loop barrier without sleeping.
    peer.handler = [](QLocalSocket* socket, const QByteArray&) {
      socket->write("{\"ok\":true,\"running\":true}\n");
    };
    WorkerControlClient next({name, {}, {}});
    QSignalSpy results(&next, &WorkerControlClient::finished);
    next.Request(Command::Status);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(completions, 0);
  }

  void ServerValidatesRequests_data() {
    QTest::addColumn<QByteArray>("bytes");
    QTest::newRow("malformed") << QByteArray("{bad}\n");
    QTest::newRow("array") << QByteArray("[]\n");
    QTest::newRow("missing-command") << QByteArray("{}\n");
    QTest::newRow("command-type") << QByteArray("{\"command\":true}\n");
    QTest::newRow("empty-command") << QByteArray("{\"command\":\"\"}\n");
  }

  void ServerValidatesRequests() {
    QFETCH(QByteArray, bytes);
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    WorkerControlServer server;
    int dispatches = 0;
    server.SetHandler([&](const QJsonObject&) {
      ++dispatches;
      return QJsonObject{{"ok", true}};
    });
    QVERIFY(server.Start(name));
    QLocalSocket socket;
    socket.connectToServer(name);
    QTRY_COMPARE(socket.state(), QLocalSocket::ConnectedState);
    socket.write(bytes);
    QByteArray response;
    QTRY_VERIFY((response += socket.readAll()).contains('\n'));
    QCOMPARE(QJsonDocument::fromJson(response).object()["error"].toObject()["code"].toString(),
             QString("invalid_request"));
    QCOMPARE(dispatches, 0);
  }

  void ServerDispatchesOneFrameAndBoundsMemory() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    WorkerControlServer server;
    int dispatches = 0;
    server.SetHandler([&](const QJsonObject&) {
      ++dispatches;
      return QJsonObject{{"ok", true}};
    });
    QVERIFY(server.Start(name));
    QLocalSocket socket;
    socket.connectToServer(name);
    QTRY_COMPARE(socket.state(), QLocalSocket::ConnectedState);
    socket.write("{\"command\":");
    QCoreApplication::processEvents();
    QCOMPARE(dispatches, 0);
    socket.write("\"reload\"}\n{\"command\":\"reload\"}\n");
    QTRY_COMPARE(dispatches, 1);
    QTRY_COMPARE(socket.state(), QLocalSocket::UnconnectedState);
    QLocalSocket oversized;
    oversized.connectToServer(name);
    QTRY_COMPARE(oversized.state(), QLocalSocket::ConnectedState);
    oversized.write(QByteArray(65537, 'x'));
    QTRY_COMPARE(oversized.state(), QLocalSocket::UnconnectedState);
    QCOMPARE(dispatches, 1);
  }

  void ServerCanBeDestroyedByItsHandler() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    auto server = std::make_unique<WorkerControlServer>();
    server->SetHandler([&](const QJsonObject&) {
      server.reset();
      return QJsonObject{{"ok", true}};
    });
    QVERIFY(server->Start(name));
    QLocalSocket socket;
    socket.connectToServer(name);
    QTRY_COMPARE(socket.state(), QLocalSocket::ConnectedState);
    socket.write("{\"command\":\"destroy\"}\n");
    QTRY_VERIFY(!server);
    QTRY_COMPARE(socket.state(), QLocalSocket::UnconnectedState);
  }

  void ServerBoundsResponsesAndRejectsUnsupportedCommands() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    WorkerControlServer server;
    QVERIFY(server.Start(name));
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Send, {{"command", "unknown"}});
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("unsupported_command"));
    server.SetHandler([](const QJsonObject&) {
      return QJsonObject{{"ok", true}, {"value", QString(65537, 'x')}};
    });
    client.Request(Command::Send, {{"command", "large-response"}});
    QTRY_COMPARE(results.size(), 2);
    QCOMPARE(Code(results), QString("response_too_large"));
  }

  void StopRejectsAnAcknowledgementThatStillClaimsRunning() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    Peer peer(name);
    peer.handler = [](QLocalSocket* socket, const QByteArray&) {
      socket->write("{\"ok\":true,\"running\":true}\n");
    };
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Stop);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("invalid_response"));
  }

  void ApplicationStartupErrorsDoNotLaunchAnotherWorker() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    WorkerControlServer server;
    const QJsonObject failure{
        {"ok", false},
        {"error", QJsonObject{{"code", "application_error"}, {"detail", "not ready"}}}};
    server.SetHandler([failure](const QJsonObject&) { return failure; });
    QVERIFY(server.Start(name));
    WorkerControlClient client(Configuration(name, directory.path()));
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Start);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("application_error"));
    QCOMPARE(Result(results)["error"].toObject()["detail"].toString(), QString("not ready"));
    QCOMPARE(QDir(directory.path()).entryList({"spawned-*"}, QDir::Files).size(), 0);
  }

  void IdlePartialRequestExpiresWithoutDispatch() {
    QTemporaryDir directory;
    const auto name = Endpoint(directory);
    WorkerControlServer server;
    int dispatches = 0;
    server.SetHandler([&](const QJsonObject&) {
      ++dispatches;
      return QJsonObject{{"ok", true}, {"running", true}};
    });
    QVERIFY(server.Start(name));
    QLocalSocket socket;
    socket.connectToServer(name);
    QTRY_COMPARE(socket.state(), QLocalSocket::ConnectedState);
    socket.write("{\"command\":");
    QTRY_COMPARE_WITH_TIMEOUT(socket.state(), QLocalSocket::UnconnectedState, 7000);
    QCOMPARE(dispatches, 0);
    WorkerControlClient client({name, {}, {}});
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Status);
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["running"].toBool());
    QCOMPARE(dispatches, 1);
  }

  void RepeatedStartUsesOneProcessAndStopWaitsForExit() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    const auto name = Endpoint(directory);
    WorkerControlClient client(Configuration(name, directory.path(), "hold-exit"));
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Start);
    client.Request(Command::Start); // Busy duplicate is ignored.
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["ok"].toBool());
    const auto pid = Result(results)["pid"].toInteger();
    QVERIFY(pid > 0);
    client.Request(Command::Start);
    QTRY_COMPARE(results.size(), 2);
    QCOMPARE(Result(results)["pid"].toInteger(), pid);
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
    client.Request(Command::Stop);
    QTRY_VERIFY(QFile::exists(files.filePath("stopped")));
    QVERIFY(client.IsBusy());
    QCOMPARE(results.size(), 2);
    client.Request(Command::Start); // Cannot reverse an in-flight stop.
    QVERIFY(Write(files.filePath("exit")));
    QTRY_COMPARE(results.size(), 3);
    QVERIFY(Result(results)["ok"].toBool());
    QVERIFY(!Result(results)["running"].toBool());
    QVERIFY(!client.IsBusy());
    client.Request(Command::Status);
    QTRY_COMPARE(results.size(), 4);
    QVERIFY(!Result(results)["running"].toBool());
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
  }

  void StopSupersedesStartBeforeOwnershipAndSocketExist() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    WorkerControlClient client(
        Configuration(Endpoint(directory), directory.path(), "before-acquire"));
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Start);
    QTRY_COMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
    client.Request(Command::Stop);
    QVERIFY(Write(files.filePath("acquire")));
    QVERIFY(Write(files.filePath("listen")));
    QTRY_VERIFY(QFile::exists(files.filePath("stopped")));
    QCOMPARE(results.size(), 0);
    QVERIFY(Write(files.filePath("exit")));
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["ok"].toBool());
    QVERIFY(!Result(results)["running"].toBool());
  }

  void DifferentControllerFindsTimedOutLaunchBeforeSocketExists() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    const auto configuration =
        Configuration(Endpoint(directory), directory.path(), "before-acquire");
    WorkerControlClient starter(configuration);
    QSignalSpy start_results(&starter, &WorkerControlClient::finished);
    starter.Request(Command::Start, {}, {}, 150);
    QTRY_COMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
    QTRY_COMPARE(start_results.size(), 1);
    QCOMPARE(Code(start_results), QString("timeout"));
    WorkerControlClient stopper(configuration);
    QSignalSpy stop_results(&stopper, &WorkerControlClient::finished);
    stopper.Request(Command::Stop, {}, {}, 5000);
    QVERIFY(stopper.IsBusy());
    QCOMPARE(stop_results.size(), 0);
    QVERIFY(Write(files.filePath("acquire")));
    QVERIFY(Write(files.filePath("listen")));
    QTRY_VERIFY(QFile::exists(files.filePath("stopped")));
    QCOMPARE(stop_results.size(), 0);
    QVERIFY(Write(files.filePath("exit")));
    QTRY_COMPARE(stop_results.size(), 1);
    QVERIFY(Result(stop_results)["ok"].toBool());
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
  }

  void StopTimeoutDoesNotClaimSuccessAndCanBeRetried_data() {
    QTest::addColumn<QString>("behavior");
    QTest::addColumn<bool>("foreground");
    QTest::newRow("detached-control-open") << QString("hold-exit") << false;
    QTest::newRow("detached-control-closed") << QString("close-before-exit") << false;
    QTest::newRow("foreground-control-open") << QString("hold-exit") << true;
    QTest::newRow("foreground-control-closed") << QString("close-before-exit") << true;
  }

  void StopTimeoutDoesNotClaimSuccessAndCanBeRetried() {
    QFETCH(QString, behavior);
    QFETCH(bool, foreground);
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    const auto configuration = Configuration(Endpoint(directory), directory.path(), behavior);
    WorkerControlClient starter(configuration);
    QSignalSpy started(&starter, &WorkerControlClient::finished);
    QProcess worker;
    if (foreground) {
      worker.start(configuration.executable, configuration.arguments);
      QVERIFY(worker.waitForStarted(5000));
      QTRY_VERIFY(QFile::exists(files.filePath("listening")));
      starter.Request(Command::Status);
    } else {
      starter.Request(Command::Start);
    }
    QTRY_COMPARE(started.size(), 1);
    QVERIFY(Result(started)["ok"].toBool());
    WorkerControlClient stopper(configuration);
    QSignalSpy results(&stopper, &WorkerControlClient::finished);
    stopper.Request(Command::Stop, {}, {}, 150);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("timeout"));
    QVERIFY(!stopper.IsBusy());
    WorkerControlClient retry(configuration);
    QSignalSpy retried(&retry, &WorkerControlClient::finished);
    retry.Request(Command::Stop);
    QTRY_VERIFY(QFile::exists(files.filePath("stopped")));
    QVERIFY(Write(files.filePath("exit")));
    QTRY_COMPARE(retried.size(), 1);
    QVERIFY(Result(retried)["ok"].toBool());
    QVERIFY(!Result(retried)["running"].toBool());
    if (foreground) {
      QVERIFY(worker.waitForFinished(0) || worker.state() == QProcess::NotRunning);
      QCOMPARE(worker.exitCode(), 0);
    }
  }

  void StartupExitIsReportedWithoutRespawnLoop() {
    QTemporaryDir directory;
    Cleanup cleanup{directory.path()};
    WorkerControlClient client(
        Configuration(Endpoint(directory), directory.path(), "exit-before-listen"));
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Start, {}, {}, 3000);
    QTRY_COMPARE(results.size(), 1);
    QCOMPARE(Code(results), QString("startup_error"));
    QCOMPARE(QDir(directory.path()).entryList({"spawned-*"}, QDir::Files).size(), 1);
  }

  void StartupReadinessIsDistinctFromSocketAvailability() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    WorkerControlClient client(Configuration(Endpoint(directory), directory.path(), "not-ready"));
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Start);
    QTRY_VERIFY(QFile::exists(files.filePath("listening")));
    QCOMPARE(results.size(), 0);
    QVERIFY(client.IsBusy());
    QVERIFY(Write(files.filePath("ready")));
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["running"].toBool());
    client.Request(Command::Stop);
    QTRY_COMPARE(results.size(), 2);
    QVERIFY(Result(results)["ok"].toBool());
  }

  void ConcurrentProcessesLaunchExactlyOneWorker() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    const auto name = Endpoint(directory);
    Cleanup cleanup{directory.path()};
    std::vector<std::unique_ptr<QProcess>> clients;
    for (int i = 0; i < 8; ++i) {
      auto client = std::make_unique<QProcess>();
      client->start(QCoreApplication::applicationFilePath(),
                    {"--client-test", "start", name, directory.path(), "gated"});
      QVERIFY(client->waitForStarted(5000));
      clients.push_back(std::move(client));
    }
    QTRY_VERIFY(QFile::exists(files.filePath("owned")));
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
    for (const auto& client : clients)
      QCOMPARE(client->state(), QProcess::Running);
    QVERIFY(Write(files.filePath("listen")));
    const auto all_finished = [&] {
      for (const auto& client : clients) {
        if (client->state() != QProcess::NotRunning)
          return false;
      }
      return true;
    };
    QTRY_VERIFY_WITH_TIMEOUT(all_finished(), 10000);
    qint64 pid = 0;
    for (const auto& client : clients) {
      QCOMPARE(client->exitCode(), 0);
      const auto result = QJsonDocument::fromJson(client->readAllStandardOutput()).object();
      QVERIFY(result["running"].toBool());
      if (pid == 0)
        pid = result["pid"].toInteger();
      QCOMPARE(result["pid"].toInteger(), pid);
    }
    QVERIFY(pid > 0);
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
    QCOMPARE(files.entryList({"duplicate-*"}, QDir::Files).size(), 0);
    clients.clear();
    for (int i = 0; i < 8; ++i) {
      auto client = std::make_unique<QProcess>();
      client->start(QCoreApplication::applicationFilePath(),
                    {"--client-test", "stop", name, directory.path(), "gated"});
      QVERIFY(client->waitForStarted(5000));
      clients.push_back(std::move(client));
    }
    QTRY_VERIFY(QFile::exists(files.filePath("stopped")));
    for (const auto& client : clients)
      QCOMPARE(client->state(), QProcess::Running);
    QVERIFY(Write(files.filePath("exit")));
    QTRY_VERIFY_WITH_TIMEOUT(all_finished(), 10000);
    for (const auto& client : clients) {
      QCOMPARE(client->exitCode(), 0);
      const auto result = QJsonDocument::fromJson(client->readAllStandardOutput()).object();
      QVERIFY(result["ok"].toBool());
      QVERIFY(!result["running"].toBool());
    }
  }

  void SeparateStopWaitsForConcurrentStartThenStopsItsWorker() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    const auto configuration = Configuration(Endpoint(directory), directory.path(), "gated");
    WorkerControlClient starter(configuration), stopper(configuration);
    QSignalSpy starts(&starter, &WorkerControlClient::finished);
    QSignalSpy stops(&stopper, &WorkerControlClient::finished);
    starter.Request(Command::Start);
    QTRY_VERIFY(QFile::exists(files.filePath("owned")));
    stopper.Request(Command::Stop);
    QCOMPARE(stops.size(), 0);
    QVERIFY(Write(files.filePath("listen")));
    QTRY_COMPARE(starts.size(), 1);
    QVERIFY(Result(starts)["running"].toBool());
    QTRY_VERIFY(QFile::exists(files.filePath("stopped")));
    QCOMPARE(stops.size(), 0);
    QVERIFY(Write(files.filePath("exit")));
    QTRY_COMPARE(stops.size(), 1);
    QVERIFY(Result(stops)["ok"].toBool());
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
  }

  void ControllerCrashDoesNotLoseItsStartingChild() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    const auto name = Endpoint(directory);
    QProcess starter;
    starter.start(QCoreApplication::applicationFilePath(),
                  {"--client-test", "start", name, directory.path(), "before-acquire"});
    QVERIFY(starter.waitForStarted(5000));
    QTRY_COMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
    starter.kill();
    QVERIFY(starter.waitForFinished(5000));
    WorkerControlClient stopper(Configuration(name, directory.path(), "before-acquire"));
    QSignalSpy results(&stopper, &WorkerControlClient::finished);
    stopper.Request(Command::Stop);
    QVERIFY(Write(files.filePath("acquire")));
    QVERIFY(Write(files.filePath("listen")));
    QTRY_VERIFY(QFile::exists(files.filePath("stopped")));
    QCOMPARE(results.size(), 0);
    QVERIFY(Write(files.filePath("exit")));
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["ok"].toBool());
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 1);
  }

  void WorkerCrashReleasesStaleOwnershipAndSocket() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    Cleanup cleanup{directory.path()};
    const auto name = Endpoint(directory);
    QProcess worker;
    worker.start(QCoreApplication::applicationFilePath(),
                 {"--worker-test", name, directory.path(), "normal"});
    QVERIFY(worker.waitForStarted(5000));
    QTRY_VERIFY(QFile::exists(files.filePath("listening")));
    const auto old_pid = worker.processId();
    worker.kill();
    QVERIFY(worker.waitForFinished(5000));
    WorkerControlClient client(Configuration(name, directory.path()));
    QSignalSpy results(&client, &WorkerControlClient::finished);
    client.Request(Command::Start);
    QTRY_COMPARE(results.size(), 1);
    QVERIFY(Result(results)["ok"].toBool());
    QVERIFY(Result(results)["pid"].toInteger() != old_pid);
    QCOMPARE(files.entryList({"spawned-*"}, QDir::Files).size(), 2);
    client.Request(Command::Stop);
    QTRY_COMPARE(results.size(), 2);
    QVERIFY(Result(results)["ok"].toBool());
  }

  void ForegroundStartSurvivesConcurrentStatusProbes() {
    QTemporaryDir directory;
    const QDir files(directory.path());
    const auto name = Endpoint(directory);
    Cleanup cleanup{directory.path()};
    std::vector<std::unique_ptr<QProcess>> probes;
    for (int i = 0; i < 4; ++i) {
      auto probe = std::make_unique<QProcess>();
      probe->start(QCoreApplication::applicationFilePath(),
                   {"--probe-test", name, directory.path()});
      QVERIFY(probe->waitForStarted(5000));
      probes.push_back(std::move(probe));
    }
    QTRY_COMPARE(files.entryList({"probe-*"}, QDir::Files).size(), 4);
    QProcess worker;
    worker.start(QCoreApplication::applicationFilePath(),
                 {"--worker-test", name, directory.path(), "normal"});
    QVERIFY(worker.waitForStarted(5000));
    QTRY_VERIFY(QFile::exists(files.filePath("listening")));
    QCOMPARE(files.entryList({"duplicate-*"}, QDir::Files).size(), 0);
    QVERIFY(Write(files.filePath("stop-probes")));
    for (const auto& probe : probes) {
      QTRY_COMPARE(probe->state(), QProcess::NotRunning);
      QCOMPARE(probe->exitCode(), 0);
    }
    WorkerControlClient stopper(Configuration(name, directory.path()));
    QSignalSpy stopped(&stopper, &WorkerControlClient::finished);
    stopper.Request(Command::Stop);
    QTRY_COMPARE(stopped.size(), 1);
    QVERIFY(Result(stopped)["ok"].toBool());
    QVERIFY(worker.waitForFinished(0) || worker.state() == QProcess::NotRunning);
    QCOMPARE(worker.exitCode(), 0);
  }

  void DistinctEndpointsRunIndependently() {
    QTemporaryDir first_directory, second_directory;
    Cleanup first_cleanup{first_directory.path()}, second_cleanup{second_directory.path()};
    WorkerControlClient first(Configuration(Endpoint(first_directory), first_directory.path()));
    WorkerControlClient second(Configuration(Endpoint(second_directory), second_directory.path()));
    QSignalSpy first_results(&first, &WorkerControlClient::finished);
    QSignalSpy second_results(&second, &WorkerControlClient::finished);
    first.Request(Command::Start);
    second.Request(Command::Start);
    QTRY_COMPARE(first_results.size(), 1);
    QTRY_COMPARE(second_results.size(), 1);
    QVERIFY(Result(first_results)["running"].toBool());
    QVERIFY(Result(second_results)["running"].toBool());
    QVERIFY(Result(first_results)["pid"] != Result(second_results)["pid"]);
    first.Request(Command::Stop);
    QTRY_COMPARE(first_results.size(), 2);
    second.Request(Command::Status);
    QTRY_COMPARE(second_results.size(), 2);
    QVERIFY(Result(second_results)["running"].toBool());
    second.Request(Command::Stop);
    QTRY_COMPARE(second_results.size(), 3);
    QVERIFY(Result(second_results)["ok"].toBool());
  }
};

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  const auto args = app.arguments();
  if (args.value(1) == "--worker-test")
    return RunWorker(app, args.value(2), args.value(3), args.value(4));
  if (args.value(1) == "--client-test")
    return RunClient(app, args);
  if (args.value(1) == "--probe-test")
    return RunProbes(app, args.value(2), args.value(3));
  WorkerControlTests tests;
  return QTest::qExec(&tests, argc, argv);
}

#include "WorkerControlTests.moc"
