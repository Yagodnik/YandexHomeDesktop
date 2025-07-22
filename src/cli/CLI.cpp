#include "CLI.h"

#include <QGuiApplication>

#include "ListDevicesCommand.h"
#include "api/YandexHomeApi.h"
#include "auth/AuthorizationService.h"

CLI::CLI(QGuiApplication* app, YandexHomeApi* api, QObject *parent)
  : QObject(parent), app_(app), api_(api)
{
  if (app_ == nullptr) {
    return;
  }

  parser_.addHelpOption();
  parser_.addOption({ "list-devices", "list all devices available" });
  parser_.addOption({ "on_off", R"(sends "on" or "off" to on_off capability)", "device name" });
  parser_.addOption({ "info", "prints info for capability" });
  parser_.addOption({ "value", "value to set for capability", "capability value" });
  parser_.addOption({ "logs", "copies all latest logs to pwd folder" });
  parser_.addOption({ "account-info", "prints account info" });
  parser_.addOption({ "i-know-what-i-am-doing",
                     "verify that you know what you are doing (required for some commands)" });
  parser_.addOption({ "reset", "resets all app stored data" });

  parser_.process(app_->arguments());

  if (parser_.isSet("reset")) {
    HandleReset();
    return;
  }

  if (parser_.isSet("list-devices")) {
    new ListDevicesCommand(api_, app_);
    return;
  }

  if (HandleCapabilities()) {
    return;
  }
}

void CLI::HandleReset() {
  if (!parser_.isSet("i-know-what-i-am-doing")) {
    qDebug() << "Warning! --reset will reset all settings from app, specify --i-know-what-i-am-doing if know what you are doing";
    return;
  }

  qDebug() << "Resetting app stored data";

  // TODO: Do actual reset here
}

bool CLI::HandleCapabilities() {
  for (const auto& option : parser_.optionNames()) {
    if (kCapabilityExecutors.contains(option)) {
      const auto executor = kCapabilityExecutors[option](api_);

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
