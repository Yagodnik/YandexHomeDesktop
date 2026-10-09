#include "RestCommand.h"
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QSet>
#include <cstdio>
namespace {
constexpr int MaxTimeoutMs = 60 * 60 * 1000;
}
QByteArray RestError(bool json, const QString& code, const QString& message) {
  return json ? QJsonDocument(
                    QJsonObject{{"ok", false},
                                {"error", QJsonObject{{"code", code}, {"message", message}}}})
                        .toJson(QJsonDocument::Compact) +
                    '\n'
              : message.toUtf8() + '\n';
}
void RestPrint(const QByteArray& bytes, bool error) {
  auto* stream = error ? stderr : stdout;
  std::fwrite(bytes.constData(), 1, bytes.size(), stream);
  std::fflush(stream);
}

int RestOutput(const QJsonObject& result, bool json) {
  if (!result["ok"].toBool()) {
    const auto error = result["error"].toObject();
    RestPrint(RestError(json, error["code"].toString(), error["message"].toString()), true);
    const auto code = error["code"].toString();
    if (code == "timeout") {
      return RestConsole::Timeout;
    }
    if (code.startsWith("authorization_")) {
      return RestConsole::Unauthorized;
    }
    return RestConsole::RequestFailed;
  }
  if (json) {
    RestPrint(QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n');

  } else {
    RestPrint((result["running"].toBool()
                   ? RestAppMessages::tr("REST API работает: %1\n").arg(result["url"].toString())
                   : RestAppMessages::tr("REST API остановлен (включён: %1).\n")
                         .arg(result["enabled"].toBool() ? RestAppMessages::tr("да")
                                                         : RestAppMessages::tr("нет")))
                  .toUtf8());
  }
  return RestConsole::Success;
}

std::expected<RestConsoleOptions, int> ParseRestConsole(const StartupOptions& options) {
  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addOption({{"json", "j"}, "JSON"});
  parser.addOption({"no-progress", "No progress"});
  parser.addOption(
      {"timeout", "Timeout", "ms", QString::number(RestControlService::DefaultTimeoutMs)});
  bool valid_timeout = false;
  if (!parser.parse(options.cli_arguments) || !parser.positionalArguments().isEmpty()) {
    RestPrint(
        RestError(options.cli_arguments.contains("--json") || options.cli_arguments.contains("-j"),
                  "usage",
                  RestAppMessages::tr("REST-команда не допускает других команд или параметров.")),
        true);
    return std::unexpected(RestConsole::Usage);
  }
  const bool json = parser.isSet("json");
  QSet<QString> seen;
  for (const auto& supplied : parser.optionNames()) {
    const auto option = supplied == "j" ? QString("json") : supplied;
    if (seen.contains(option)) {
      RestPrint(RestError(json, "usage",
                          QCoreApplication::translate("CliCommand", "Повторяющийся параметр: --%1.")
                              .arg(supplied)),
                true);
      return std::unexpected(RestConsole::Usage);
    }
    seen.insert(option);
  }
  const auto timeout_ms = parser.value("timeout").toInt(&valid_timeout);
  if (!valid_timeout || timeout_ms < 1 || timeout_ms > MaxTimeoutMs) {
    RestPrint(RestError(json, "usage",
                        RestAppMessages::tr("--timeout должен быть от 1 до 3600000 миллисекунд.")),
              true);
    return std::unexpected(RestConsole::Usage);
  }
  if (parser.isSet("help") || parser.isSet("help-all")) {
    const auto help =
        options.cli_arguments.value(0) +
        " [--fake-api] [--fake-api-data <path>] [--rest-port <port>] [--json] [--timeout <ms>]\n"
        "  --enable-rest  --disable-rest  --status-rest\n" +
        RestAppMessages::tr("YandexHomeRest runs the server in the foreground by default.") + "\n";
    RestPrint(json ? QJsonDocument(QJsonObject{{"ok", true}, {"help", help}})
                             .toJson(QJsonDocument::Compact) +
                         '\n'
                   : help.toUtf8());
    return std::unexpected(RestConsole::Success);
  }
  return RestConsoleOptions{json, timeout_ms};
}

int RunRestControl(QCoreApplication& app, const StartupOptions& options) {
  const auto parsed = ParseRestConsole(options);
  if (!parsed)
    return parsed.error();
  Settings settings(nullptr, options.use_fake_api);
  RestControlService service(&settings, RestConfiguration(options));
  auto command = RestControlService::Command::Status;
  if (options.rest_command == StartupOptions::RestCommand::Enable)
    command = RestControlService::Command::Enable;
  if (options.rest_command == StartupOptions::RestCommand::Disable)
    command = RestControlService::Command::Disable;
  QObject::connect(&service, &RestControlService::finished, &app,
                   [&](const QJsonObject& result) { app.exit(RestOutput(result, parsed->json)); });
  QTimer::singleShot(0, &app,
                     [&] { service.Request(command, options.rest_port, parsed->timeout_ms); });
  return app.exec();
}
