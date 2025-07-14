#include "CLI.h"

#include <QGuiApplication>

CLI::CLI(QCoreApplication* app, QObject *parent) : QObject(parent), app_(app) {
  if (app_ == nullptr) {
    return;
  }

  parser_.addHelpOption();

  const QCommandLineOption list_devices_option("list-devices", "list all devices available");
  parser_.addOption(list_devices_option);

  const QCommandLineOption on_off_option("on_off", R"(sends "on" or "off" to on_off capability)", "device name");
  parser_.addOption(on_off_option);

  const QCommandLineOption info_option("info", "prints info for capability");
  parser_.addOption(info_option);

  const QCommandLineOption value_option("value", "value to set for capability", "capability value");
  parser_.addOption(value_option);

  const QCommandLineOption dump_logs_option("logs", "copies all latest logs to pwd folder");
  parser_.addOption(dump_logs_option);

  const QCommandLineOption account_info_option("account-info", "prints account info");
  parser_.addOption(account_info_option);

  const QCommandLineOption pro_mode_option("i-know-what-i-am-doing", "verify that you know what you are doing (required for some commands)");
  parser_.addOption(pro_mode_option);

  const QCommandLineOption reset_option("reset", "resets all app stored data");
  parser_.addOption(reset_option);

  parser_.process(app_->arguments());

  if (parser_.isSet(reset_option)) {
    if (!parser_.isSet(pro_mode_option)) {
      qDebug() << "Warning! --reset will reset all settings from app, specify --i-know-what-i-am-doing if know what you are doing";
      return;
    }

    qDebug() << "Resetting app stored data";

    // TODO: Do actual reset here

    return;
  }

  if (HandleCapabilities()) {
    return;
  }
}

bool CLI::HandleCapabilities() const {
  for (const auto& option : parser_.optionNames()) {
    if (kCapabilityExecutors.contains(option)) {
      const auto executor = kCapabilityExecutors[option]();

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
