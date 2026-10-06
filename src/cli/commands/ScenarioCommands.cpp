#include "ScenarioCommands.h"

#include <QJsonArray>
#include "cli/CliContext.h"

namespace {
class ListScenariosCommand final : public ICommand {
public:
  void Execute(CliContext& context) const override {
    context.Services().scenarios->ListScenarios(context.Owner(), [&context](ApiResult<QList<ScenarioObject>> result) {
      if (!result) { context.FailApi(result.error()); return; }
      QJsonArray scenarios;
      QString text = ScenarioCommands::tr("ID\tИмя\tАктивен\n");
      for (const auto& scenario : *result) {
        scenarios.append(QJsonObject{{"id", scenario.id}, {"name", scenario.name}, {"is_active", scenario.is_active}});
        text += QString("%1\t%2\t%3\n").arg(scenario.id, scenario.name, scenario.is_active ? "true" : "false");
      }
      context.Complete({{"scenarios", scenarios}}, text);
    });
  }
};
class RunScenarioCommand final : public ICommand {
public:
  explicit RunScenarioCommand(CliTarget target) : target_(std::move(target)) {}
  static CommandRegistry::Result Parse(const CliArguments& arguments) {
    const auto target = CliTarget::Parse(arguments, false);
    if (!target) { return std::unexpected(target.error()); }
    return std::make_shared<RunScenarioCommand>(*target);
  }
  void Execute(CliContext& context) const override {
    context.Services().scenarios->ListScenarios(context.Owner(), [this, &context](ApiResult<QList<ScenarioObject>> result) {
      if (!result) { context.FailApi(result.error()); return; }
      QJsonArray matches;
      for (const auto& scenario : *result) {
        if ((!target_.id.isEmpty() && scenario.id == target_.id) || (!target_.name.isEmpty() && scenario.name == target_.name)) {
          matches.append(QJsonObject{{"id", scenario.id}, {"name", scenario.name}, {"is_active", scenario.is_active}});
        }
      }
      if (matches.isEmpty()) { context.Fail(CliContext::NotFound, "scenario_not_found", ScenarioCommands::tr("Сценарий не найден.")); return; }
      if (matches.size() > 1) { context.Fail(CliContext::Usage, "ambiguous_target", ScenarioCommands::tr("Найдено несколько сценариев. Укажите --id."), {{"matches", matches}}); return; }
      const auto scenario = matches[0].toObject();
      if (!scenario["is_active"].toBool()) { context.Fail(CliContext::Usage, "inactive_scenario", ScenarioCommands::tr("Сценарий неактивен.")); return; }
      const auto id = scenario["id"].toString();
      context.Services().scenarios->RunScenario(id, context.Owner(), [&context, id](ApiResult<void> result) {
        if (!result) { context.FailApi(result.error()); return; }
        context.Complete({{"scenario_id", id}}, ScenarioCommands::tr("Сценарий %1 выполнен.\n").arg(id));
      });
    });
  }
private:
  CliTarget target_;
};
}
void RegisterScenarioCommands(CommandRegistry& registry) {
  AddTargetOptions(registry);
  registry.Register({{"scenarios", "list"}, ScenarioCommands::tr("Список сценариев с ID."), {},
    [](const CliArguments&) -> CommandRegistry::Result { return std::make_shared<ListScenariosCommand>(); }});
  registry.Register({{"scenarios", "run"}, ScenarioCommands::tr("Выполнить активный сценарий."),
    {"id", "name"}, RunScenarioCommand::Parse});
}
