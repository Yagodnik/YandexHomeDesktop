#include "StartupOptions.h"

std::expected<StartupOptions, QString> ParseStartupOptions(const QStringList& arguments, bool allow_fake_api) {
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
  return options;
}
