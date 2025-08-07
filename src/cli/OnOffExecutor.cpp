#include "OnOffExecutor.h"

#include <iostream>
#include <QDebug>
#include <QGuiApplication>

#include "iot/capabilities/OnOffCapability.h"

OnOffExecutor::OnOffExecutor(YandexHomeApi *api, QObject *parent) : IExecutor(api, parent) {
  connect(api_,
    &YandexHomeApi::userInfoReceived,
    this,
    &OnOffExecutor::OnUserInfoReceived);

  connect(api_,
    &YandexHomeApi::userInfoReceivingFailed,
    this,
    &OnOffExecutor::OnUserInfoReceivingFailed);

  connect(api_,
    &YandexHomeApi::actionExecutingFinishedSuccessfully,
    this,
    &OnOffExecutor::OnActionExecutionFinishedSuccessfully);

  connect(api_,
    &YandexHomeApi::actionExecutingFailed,
    this,
    &OnOffExecutor::OnActionExecutionFailed);
}

void OnOffExecutor::Execute(const QString& name, const QString& value) {
  if (value == "on") {
    value_ = true;
  } else if (value == "off") {
    value_ = false;
  } else {
    std::cout << "Incorrect value for OnOff: " << value.toStdString() << std::endl;
    QGuiApplication::exit(0);
  }

  target_device_name_ = name;
  api_->GetUserInfo();
}

void OnOffExecutor::PrintInfo() {
  std::cout << "OnOff Capability" << std::endl;
  std::cout << "Usage: --on_off \"name here\" --value on " << std::endl;
  std::cout << "       --on_off \"name here\" --value off " << std::endl;

  QGuiApplication::exit(0);
}

void OnOffExecutor::OnUserInfoReceived(const UserInfo &userInfo) {
  for (const auto& device : userInfo.devices) {
    if (device.name == target_device_name_) {
      target_device_id_ = device.id;
      std::cout << "Found device with name \"" << target_device_name_.toStdString() << "\""  << std::endl;

      OnOffCapability capability;
      auto state = capability.Create(value_);

      const CapabilityObject action = {
        .type = CapabilityType::OnOff,
        .state = state
      };

      const DeviceActionsObject action_object = {
        .id = target_device_id_,
        .actions = { action }
      };

      api_->PerformActions({ action_object }, {});

      return;
    }
  }

  std::cout << "Unable to find device with name \"" << target_device_name_.toStdString() << "\""  << std::endl;

  QGuiApplication::exit(0);
}

void OnOffExecutor::OnUserInfoReceivingFailed(const QString& message) {
  std::cout << "Failed to find device: " << message.toStdString() << std::endl;

  QGuiApplication::exit(0);
}

void OnOffExecutor::OnActionExecutionFinishedSuccessfully(const QVariant &user_data) {
  std::cout << "Action on device \"" << target_device_name_.toStdString()
            << "\" finished - OK" << std::endl;

  QGuiApplication::exit(0);
}

void OnOffExecutor::OnActionExecutionFailed(const QString &message, const QVariant &user_data) {
  std::cout << "Action on device \"" << target_device_name_.toStdString()
            << "\" finished - FAIL" << std::endl;

  QGuiApplication::exit(0);
}
