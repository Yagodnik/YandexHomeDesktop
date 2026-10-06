#include "CapabilityCommands.h"
#include "capabilities/Builtins.h"
#include "iot/core/CapabilityState.h"

void CapabilityCommands::Register(const QString& name, std::shared_ptr<const ICapabilityInput> input) {
  inputs_[name] = std::move(input);
}

std::expected<CapabilityAction, QString> CapabilityCommands::Parse(const CliArguments& arguments) const {
  const auto name = arguments.Value("capability");
  const auto prefix = QStringLiteral("devices.capabilities.");
  const auto input = inputs_.value(name.startsWith(prefix) ? name.mid(prefix.size()) : name);
  if (!input || !arguments.Has("value")) {
    return std::unexpected(tr("Укажите поддерживаемый --capability и --value."));
  }

  auto state = input->Parse(arguments);
  if (!state) { return std::unexpected(state.error()); }
  if (arguments.Has("relative")) { *state = Iot::State::WithRelative(*state); }
  const auto rules = input->Rules();
  const auto valid = Iot::ValidateInput(*rules, *state);
  if (!valid) { return std::unexpected(valid.error()); }
  return CapabilityAction{rules, *state};
}

CapabilityCommands CapabilityCommands::Builtin() {
  CapabilityCommands registry;
  registry.Register("on_off", CliCapability::OnOff());
  registry.Register("toggle", CliCapability::Toggle());
  registry.Register("range", CliCapability::Range());
  registry.Register("mode", CliCapability::Mode());
  registry.Register("color_setting", CliCapability::Color());
  return registry;
}
