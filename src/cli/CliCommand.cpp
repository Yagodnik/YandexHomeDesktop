#include "CliCommand.h"
#include <QSet>
#include "commands/LocalCommands.h"

namespace {
std::expected<CliArguments, QString> ReadOptions(const QCommandLineParser& parser, const CommandRegistry& registry) {
  CliArguments arguments;
  QSet<QString> seen;
  for (const auto& supplied_name : parser.optionNames()) {
    const auto name = registry.CanonicalOption(supplied_name);
    if (seen.contains(name)) {
      return std::unexpected(CliCommand::tr("Повторяющийся параметр: --%1.").arg(supplied_name));
    }
    seen.insert(name);
    arguments.options[name] = parser.value(supplied_name);
  }
  return arguments;
}

std::expected<int, QString> ReadTimeout(const QCommandLineParser& parser) {
  if (!parser.isSet("timeout")) { return 30000; }
  bool is_integer = false;
  const auto timeout = parser.value("timeout").toInt(&is_integer);
  if (!is_integer || timeout < 1 || timeout > 3600000) {
    return std::unexpected(CliCommand::tr("--timeout должен быть от 1 до 3600000 миллисекунд."));
  }
  return timeout;
}
}

QString CliCommand::Help(const QString& program, const CommandRegistry& registry) {
  return registry.Help(program);
}

std::expected<CliCommand, QString> CliCommand::Parse(const QStringList& arguments, const CommandRegistry& registry) {
  QCommandLineParser parser;
  registry.Configure(parser);
  if (!parser.parse(arguments)) { return std::unexpected(parser.errorText()); }

  CliCommand invocation;
  invocation.json = parser.isSet("json");
  if (parser.isSet("help") || parser.isSet("help-all")) {
    invocation.operation = std::make_shared<HelpCommand>(registry.Help(arguments.value(0)));
    return invocation;
  }

  auto options = ReadOptions(parser, registry);
  if (!options) { return std::unexpected(options.error()); }
  const auto timeout = ReadTimeout(parser);
  if (!timeout) { return std::unexpected(timeout.error()); }

  options->options.remove("json");
  options->options.remove("timeout");
  const auto operation = registry.Create(parser.positionalArguments(), *options);
  if (!operation) { return std::unexpected(operation.error()); }

  invocation.timeout_ms = *timeout;
  invocation.operation = *operation;
  return invocation;
}
