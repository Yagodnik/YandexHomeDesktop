#include "StartupOptions.h"
#include <QCoreApplication>
#include <QFileInfo>

class StartupMessages {
  Q_DECLARE_TR_FUNCTIONS(StartupMessages)
};

std::expected<StartupOptions, QString> ParseStartupOptions(const QStringList& arguments,
                                                           bool allow_fake_api, bool rest_worker,
                                                           bool allow_fake_auth) {
  StartupOptions options;
  bool custom_fixture = false;
  if (!arguments.isEmpty()) {
    options.cli_arguments.append(arguments.first());
  }
  for (int i = 1; i < arguments.size(); ++i) {
    const auto& argument = arguments[i];
    if (argument == "--") {
      options.cli_arguments.append(arguments.mid(i));
      break;
    } else if (argument == "--fake-auth") {
      options.use_fake_auth = true;
    } else if (argument == "--fake-api") {
      options.use_fake_api = true;
    } else if (argument == "--mcp-port" || argument.startsWith("--mcp-port=")) {
      if (options.mcp_port) return std::unexpected(StartupMessages::tr("Повторяющийся параметр: --mcp-port."));
      const auto value = argument == "--mcp-port" ? arguments.value(++i) : argument.mid(QString("--mcp-port=").size());
      bool valid = false;
      const auto port = value.toUInt(&valid);
      if (!valid || port == 0 || port > 65535) return std::unexpected(StartupMessages::tr("--mcp-port должен быть от 1 до 65535."));
      options.mcp_port = static_cast<quint16>(port);
    } else if (argument == "--enable-mcp" || argument == "--disable-mcp" || argument == "--status-mcp") {
      if (options.mcp_command != StartupOptions::McpCommand::None)
        return std::unexpected(StartupMessages::tr("Укажите только одну MCP-команду."));
      options.mcp_command = argument == "--enable-mcp" ? StartupOptions::McpCommand::Enable
                            : argument == "--disable-mcp" ? StartupOptions::McpCommand::Disable
                                                          : StartupOptions::McpCommand::Status;
    } else if (argument == "--enable-rest" || argument == "--disable-rest" ||
               argument == "--status-rest" || argument == "--serve-rest") {
      if (options.rest_command != StartupOptions::RestCommand::None) {
        return std::unexpected(StartupMessages::tr("Укажите только одну REST-команду."));
      }
      if (argument == "--enable-rest")
        options.rest_command = StartupOptions::RestCommand::Enable;
      if (argument == "--disable-rest")
        options.rest_command = StartupOptions::RestCommand::Disable;
      if (argument == "--status-rest")
        options.rest_command = StartupOptions::RestCommand::Status;
      if (argument == "--serve-rest")
        options.rest_command = StartupOptions::RestCommand::Serve;
    } else if (argument == "--rest-config" || argument.startsWith("--rest-config=")) {
      if (!options.rest_config_file.isEmpty()) {
        return std::unexpected(StartupMessages::tr("Duplicate option: --rest-config."));
      }
      const auto path = argument == "--rest-config"
                            ? arguments.value(++i)
                            : argument.mid(QString("--rest-config=").size());
      if (path.isEmpty() || path.startsWith("--")) {
        return std::unexpected(StartupMessages::tr("--rest-config requires a JSON file path."));
      }
      options.rest_config_file = QFileInfo(path).absoluteFilePath();
    } else if (argument == "--rest-port" || argument.startsWith("--rest-port=")) {
      if (options.rest_port)
        return std::unexpected(StartupMessages::tr("Повторяющийся параметр: --rest-port."));
      const QString value = argument == "--rest-port"
                                ? arguments.value(++i)
                                : argument.mid(QString("--rest-port=").size());
      bool valid = false;
      const auto port = value.toUInt(&valid);
      if (!valid || port == 0 || port > 65535) {
        return std::unexpected(StartupMessages::tr("--rest-port должен быть от 1 до 65535."));
      }
      options.rest_port = static_cast<quint16>(port);
    } else if (argument == "--fake-api-data" || argument.startsWith("--fake-api-data=")) {
      custom_fixture = true;
      if (argument == "--fake-api-data") {
        if (++i == arguments.size() || arguments[i].startsWith("--")) {
          return std::unexpected("--fake-api-data requires a JSON file path");
        }
        options.fixture_path = arguments[i];
      } else {
        options.fixture_path = argument.mid(QString("--fake-api-data=").size());
      }
      if (options.fixture_path.isEmpty()) {
        return std::unexpected("--fake-api-data requires a JSON file path");
      }
    } else {
      options.cli_arguments.append(argument);
    }
  }
  if ((options.use_fake_api || options.use_fake_auth || custom_fixture) && !allow_fake_api) {
    return std::unexpected("The fake API is available only in Debug builds");
  }
  if (options.use_fake_auth && !allow_fake_auth) {
    return std::unexpected("--fake-auth is available only in the desktop app");
  }
  if (options.use_fake_auth && !options.use_fake_api) {
    return std::unexpected("--fake-auth requires --fake-api");
  }
  if (custom_fixture && !options.use_fake_api) {
    return std::unexpected("--fake-api-data requires --fake-api");
  }
  if (options.mcp_command != StartupOptions::McpCommand::None &&
      options.rest_command != StartupOptions::RestCommand::None)
    return std::unexpected(StartupMessages::tr("Укажите только одну команду сервера."));
  if (options.mcp_port && options.mcp_command != StartupOptions::McpCommand::Enable)
    return std::unexpected(StartupMessages::tr("--mcp-port требует --enable-mcp."));
  if (options.rest_port && options.rest_command != StartupOptions::RestCommand::Enable &&
      options.rest_command != StartupOptions::RestCommand::Serve &&
      !(rest_worker && options.rest_command == StartupOptions::RestCommand::None)) {
    return std::unexpected(
        StartupMessages::tr("--rest-port требует --enable-rest или --serve-rest."));
  }
  if (!options.rest_config_file.isEmpty() &&
      options.rest_command == StartupOptions::RestCommand::None && !rest_worker) {
    return std::unexpected(
        StartupMessages::tr("--rest-config requires a REST command or YandexHomeRest."));
  }
  return options;
}
