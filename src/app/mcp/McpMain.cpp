#include <QCoreApplication>
#include <QTimer>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <cstdio>
#include "mcp/McpHttpServer.h"
#include "McpConfiguration.h"
#include "worker/WorkerControlClient.h"
#include "worker/WorkerControlServer.h"

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  app.setApplicationName("YandexHomeMcp");
  if (app.arguments().contains("--help") && app.arguments().contains("--json")) {
    const auto help =
        QJsonDocument(QJsonObject{{"ok", true},
                                  {"help", "YandexHomeMcp [--fake-api] [--fake-api-data <path>]"}})
            .toJson(QJsonDocument::Compact) +
        '\n';
    std::fwrite(help.constData(), 1, help.size(), stdout);
    return 0;
  }
  if (app.arguments().contains("--help")) {
    std::fputs("YandexHomeMcp [--fake-api] [--fake-api-data <path>]\n", stdout);
    return 0;
  }
  const auto options = ParseStartupOptions(app.arguments(), true);
  if (!options || options->rest_command != StartupOptions::RestCommand::None ||
      options->mcp_command != StartupOptions::McpCommand::None ||
      options->cli_arguments.size() != 1)
    return 2;
  const auto configuration = MakeMcpConfiguration(*options);
  const auto& gui = configuration.gui_endpoint;
  const auto& worker = configuration.worker.control_name;
  quint16 port = 0;

  WorkerControlServer control;
  if (!control.Acquire(worker))
    return 1;
  QByteArray secret;
  for (int i = 0; i < 8; ++i) {
    const auto value = QRandomGenerator::system()->generate();
    secret.append(reinterpret_cast<const char*>(&value), sizeof(value));
  }
  const auto token = secret.toHex();
  McpHttpServer server(gui, token);
  WorkerControlClient gui_client({gui, {}, {}, "stop"});
  bool running = false;
  bool stopping = false;
  control.SetHandler([&](const QJsonObject& request) {
    if (request.value("command") != "stop" && request.value("command") != "status")
      return QJsonObject{{"ok", false}, {"error", QJsonObject{{"code", "unsupported_command"}}}};
    if (request.value("command") == "stop") {
      stopping = true;
      running = false;
      server.Stop();
      QTimer::singleShot(100, &app, &QCoreApplication::quit);
    }
    return QJsonObject{{"ok", true},
                       {"running", running},
                       {"port", static_cast<int>(port)},
                       {"url", QString("http://127.0.0.1:%1/mcp").arg(port)},
                       {"token", QString::fromLatin1(token)}};
  });
  if (!control.Listen())
    return 1;
  QObject::connect(&gui_client, &WorkerControlClient::finished, &app,
                   [&](const QJsonObject& result) {
                     if (stopping)
                       return;
                     if (!result.value("ok").toBool() || !result.value("running").toBool()) {
                       server.Stop();
                       app.exit(1);
                       return;
                     }
                     if (!running) {
                       const auto gui_port = result.value("port").toInt();
                       if (gui_port < 1 || gui_port > 65535) {
                         app.exit(1);
                         return;
                       }
                       port = static_cast<quint16>(gui_port);
                       if (!server.Start(port)) {
                         app.exit(1);
                         return;
                       }
                       running = true;
                     }
                   });
  QTimer heartbeat;
  heartbeat.setInterval(5'000);
  QObject::connect(&heartbeat, &QTimer::timeout, &app, [&] {
    if (!gui_client.IsBusy())
      gui_client.Request(WorkerControlClient::Command::Send, {{"command", "status"}}, {}, 3'000);
  });
  QTimer::singleShot(0, &app, [&] {
    gui_client.Request(WorkerControlClient::Command::Send, {{"command", "status"}}, {}, 3'000);
    heartbeat.start();
  });
  return app.exec();
}
