#include "CliArguments.h"
#include "CommandRegistry.h"

std::expected<CliTarget, QString> CliTarget::Parse(const CliArguments& arguments,
                                                   bool allow_household) {
  const bool has_id = arguments.Has("id");
  const bool has_name = arguments.Has("name");
  if (has_id == has_name) {
    return std::unexpected(CliArguments::tr("Укажите ровно один непустой --id или --name."));
  }

  CliTarget target{arguments.Value("id"), arguments.Value("name"), arguments.Value("household")};
  if ((has_id && target.id.isEmpty()) || (has_name && target.name.isEmpty())) {
    return std::unexpected(CliArguments::tr("Укажите ровно один непустой --id или --name."));
  }
  if (!arguments.Has("household")) {
    return target;
  }
  if (!allow_household || target.household.isEmpty()) {
    return std::unexpected(CliArguments::tr("ID дома не должен быть пустым."));
  }
  if (has_id) {
    return std::unexpected(
        CliArguments::tr("--household применяется к списку или поиску по имени, а не к --id."));
  }
  return target;
}

void AddTargetOptions(CommandRegistry& registry) {
  registry.AddOption(
      {{"id", "device-id"}, CliArguments::tr("Точный ID устройства или сценария."), "id"});
  registry.AddOption({{"name", "device-name"},
                      CliArguments::tr("Точное имя; неоднозначные имена отклоняются."),
                      "name"});
}
