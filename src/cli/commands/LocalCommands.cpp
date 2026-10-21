#include "LocalCommands.h"
#include "cli/CliContext.h"

namespace {
class AccountCommand final : public ICommand {
public:
  void Execute(CliContext& context) const override {
    context.Services().account->ReadAccount(
        context.Owner(), [&context](ApiResult<AccountInfo> result) {
          if (!result) {
            context.FailApi(result.error());
            return;
          }
          context.Complete({{"account", QJsonObject{{"name", result->display_name},
                                                    {"email", result->default_email}}}},
                           LocalCommands::tr("Имя: %1\nEmail: %2\n")
                               .arg(result->display_name, result->default_email));
        });
  }
};
class ResetCommand final : public ICommand {
public:
  static CommandRegistry::Result Parse(const CliArguments& arguments) {
    if (!arguments.Has("i-know-what-i-am-doing")) {
      return std::unexpected(LocalCommands::tr("Для сброса укажите --i-know-what-i-am-doing."));
    }
    return std::make_shared<ResetCommand>();
  }
  bool RequiresAuthorization() const override {
    return false;
  }
  void Execute(CliContext& context) const override {
    context.Services().reset(context.Owner(), [&context](ApiResult<void> result) {
      if (!result) {
        context.FailApi(result.error());
        return;
      }
      context.Complete({{"reset", true}},
                       LocalCommands::tr("Настройки сброшены; выход из аккаунта выполнен.\n"));
    });
  }
};
} // namespace
void HelpCommand::Execute(CliContext& context) const {
  context.Complete({{"help", text_}}, text_);
}
void RegisterLocalCommands(CommandRegistry& registry) {
  registry.AddOption({"i-know-what-i-am-doing",
                      LocalCommands::tr("Подтверждение сброса настроек и выхода из аккаунта.")});
  registry.AddOption({"account-info", LocalCommands::tr("Совместимость: account show.")});
  registry.AddOption({"reset", LocalCommands::tr("Совместимость: reset.")});
  registry.Register({{"account", "show"},
                     LocalCommands::tr("Имя и email аккаунта."),
                     {},
                     [](const CliArguments&) -> CommandRegistry::Result {
                       return std::make_shared<AccountCommand>();
                     }});
  registry.Register({{"reset"},
                     LocalCommands::tr("Сбросить настройки и выйти из аккаунта."),
                     {"i-know-what-i-am-doing"},
                     ResetCommand::Parse});
  registry.RegisterLegacy({"account-info", {"account", "show"}, {}, {}, {}, {}});
  registry.RegisterLegacy({"reset", {"reset"}, {}, {}, {}, {}});
}
