#include "CLI.h"

#include <QGuiApplication>
#include <unistd.h>

#include "AccountInfoCommand.h"
#include "ListDevicesCommand.h"
#include "api/YandexHomeApi.h"
#include "auth/AuthorizationService.h"

CLI::CLI(AppContext& app_ctx,  QObject *parent) :
  QObject(parent),
  app_ctx_(app_ctx)
{
  if (app_ctx_.app_ == nullptr) {
    return;
  }

  commands_list_["reset"] = new ResetCommand(parent);
  commands_list_["account-info"] = new AccountInfoCommand(parent);
  commands_list_["list-devices"] =new ListDevicesCommand(parent);

  parser_.addHelpOption();
  parser_.addOption({"list-devices", "list all devices available"});
  parser_.addOption({"on_off", R"(sends "on" or "off" to on_off capability)", "device name"});
  parser_.addOption({"info", "prints info for capability"});
  parser_.addOption({"value", "value to set for capability", "capability value"});
  parser_.addOption({"logs", "copies all latest logs to pwd folder"});
  parser_.addOption({"account-info", "prints account info"});
  parser_.addOption({
    "i-know-what-i-am-doing",
    "verify that you know what you are doing (required for some commands)"
  });
  parser_.addOption({"reset", "resets all app stored data"});

  parser_.process(app_ctx_.app_->arguments());

  CommandContext command_ctx{
    .pro_mode = parser_.isSet("i-know-what-i-am-doing")
  };

  for (auto it = commands_list_.begin(); it != commands_list_.end(); ++it) {
    const auto& key = it.key();
    const auto& command = it.value();

    if (parser_.isSet(key)) {
      command->Execute(app_ctx_, command_ctx);
      return;
    }
  }

  if (HandleCapabilities()) {
    return;
  }

  qDebug() << "Unknown command!";
  app_ctx_.app_->quit();
}

bool CLI::HandleCapabilities() {
  for (const auto& option : parser_.optionNames()) {
    if (kCapabilityExecutors.contains(option)) {
      const auto executor = kCapabilityExecutors[option](app_ctx_.yandex_api);

      if (parser_.isSet("info")) {
        executor->PrintInfo();
      } else {
        const QString capability_value = parser_.value("value");
        executor->Execute(capability_value);
      }

      return true;
    }
  }

  return false;
}
