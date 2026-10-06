#include "CommandRegistry.h"
#include <algorithm>
#include "commands/DeviceCommands.h"
#include "commands/ScenarioCommands.h"
#include "commands/LocalCommands.h"

void CommandRegistry::AddOption(const QCommandLineOption& option) {
  if (std::none_of(options_.begin(), options_.end(), [&](const auto& existing) {
      return existing.names().first() == option.names().first(); })) {
    options_.append(option);
  }
}
void CommandRegistry::Register(Definition definition) { commands_.append(std::move(definition)); }
void CommandRegistry::RegisterLegacy(LegacyAlias alias) { legacy_.append(std::move(alias)); }
QString CommandRegistry::CanonicalOption(const QString& name) const {
  if (name == "j") { return "json"; }
  for (const auto& option : options_) {
    if (option.names().contains(name)) {
      return option.names().first();
    }
  }
  return name;
}
void CommandRegistry::Configure(QCommandLineParser& parser) const {
  parser.setApplicationDescription(tr("Управление устройствами и сценариями Яндекс Дома."));
  parser.addHelpOption();
  parser.addPositionalArgument("command", tr("Команда из списка ниже."));
  parser.addOption({{"json", "j"}, tr("JSON в stdout; ошибки JSON в stderr.")});
  parser.addOption({"timeout", tr("Общий таймаут в миллисекундах (по умолчанию 30000)."), "ms"});
  for (const auto& option : options_) { parser.addOption(option); }
}
QString CommandRegistry::Describe() const {
  QString result = "\n" + tr("Команды:") + "\n";
  for (const auto& command : commands_) {
    result += "  " + command.path.join(' ') + "\t" + command.description + "\n";
  }
  return result;
}
QString CommandRegistry::Help(const QString& program) const {
  QCommandLineParser parser;
  Configure(parser);
  parser.parse({program});
  return parser.helpText() + Describe();
}
std::expected<CommandRegistry::ResolvedCommand, QString> CommandRegistry::ResolveLegacy(
    QStringList path, CliArguments arguments) const {
  const LegacyAlias* selected = nullptr;
  for (const auto& alias : legacy_) {
    if (!arguments.Has(alias.option)) { continue; }
    if (selected || !path.isEmpty()) {
      return std::unexpected(tr("Укажите только одну команду."));
    }
    selected = &alias;
  }
  if (!selected) { return ResolvedCommand{std::move(path), std::move(arguments)}; }

  for (const auto& option : selected->forbidden) {
    if (arguments.Has(option)) {
      return std::unexpected(tr("Параметр --%1 несовместим со старой командой.").arg(option));
    }
  }
  const auto value = arguments.Value(selected->option);
  arguments.options.remove(selected->option);
  if (!selected->value_option.isEmpty()) { arguments.options[selected->value_option] = value; }
  for (auto option = selected->defaults.begin(); option != selected->defaults.end(); ++option) {
    arguments.options[option.key()] = option.value();
  }
  const bool help = !selected->help_option.isEmpty() && arguments.Has(selected->help_option);
  return ResolvedCommand{selected->path, std::move(arguments), help};
}

const CommandRegistry::Definition* CommandRegistry::Find(const QStringList& path) const {
  const auto found = std::find_if(commands_.begin(), commands_.end(), [&](const auto& command) {
    return command.path == path;
  });
  return found == commands_.end() ? nullptr : &*found;
}

std::expected<void, QString> CommandRegistry::ValidateOptions(const Definition& command, const CliArguments& arguments) {
  for (auto option = arguments.options.begin(); option != arguments.options.end(); ++option) {
    if (!command.options.contains(option.key())) {
      return std::unexpected(tr("Недопустимый параметр для команды: --%1.").arg(option.key()));
    }
  }
  return {};
}

CommandRegistry::Result CommandRegistry::Create(QStringList path, CliArguments arguments) const {
  const auto resolved = ResolveLegacy(std::move(path), std::move(arguments));
  if (!resolved) { return std::unexpected(resolved.error()); }
  if (resolved->help) {
    return std::shared_ptr<const ICommand>(std::make_shared<HelpCommand>(Help(QCoreApplication::applicationName())));
  }

  const auto* command = Find(resolved->path);
  if (!command) { return std::unexpected(tr("Неизвестная команда. Используйте --help.")); }
  const auto valid = ValidateOptions(*command, resolved->arguments);
  if (!valid) { return std::unexpected(valid.error()); }
  return command->factory(resolved->arguments);
}

CommandRegistry CommandRegistry::Builtin() {
  CommandRegistry registry;
  RegisterDeviceCommands(registry);
  RegisterScenarioCommands(registry);
  RegisterLocalCommands(registry);
  return registry;
}
