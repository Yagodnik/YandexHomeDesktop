#include "StartupOptions.h"
#include <QCoreApplication>

class StartupMessages {
  Q_DECLARE_TR_FUNCTIONS(StartupMessages)
};

std::expected<StartupOptions, QString> ParseStartupOptions(const QStringList& arguments,
                                                           bool allow_fake_api, bool rest_worker) {
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
    } else if (argument == "--fake-api") {
      options.use_fake_api = true;
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
  if ((options.use_fake_api || custom_fixture) && !allow_fake_api) {
    return std::unexpected("The fake API is available only in Debug builds");
  }
  if (custom_fixture && !options.use_fake_api) {
    return std::unexpected("--fake-api-data requires --fake-api");
  }
  if (options.rest_port && options.rest_command != StartupOptions::RestCommand::Enable &&
      options.rest_command != StartupOptions::RestCommand::Serve &&
      !(rest_worker && options.rest_command == StartupOptions::RestCommand::None)) {
    return std::unexpected(
        StartupMessages::tr("--rest-port требует --enable-rest или --serve-rest."));
  }
  return options;
}
