#include "DeviceViewModel.h"

DeviceViewModel::DeviceViewModel(QObject *parent) :
  QObject(parent),
  state_(std::make_unique<DeviceViewState>())
{
}

DeviceViewState* DeviceViewModel::GetState() const {
  return state_.get();
}

void DeviceViewModel::ShowErrorMessageForDevice(const QString& error_code) const {
  const auto error = error_codes_->GetDeviceError(error_code).toMap();

  if (!error.contains(kShortDescriptionKey) || !error.contains(kFullDescriptionKey)) {
    ShowErrorMessage(tr(kErrorOccurredMessage));
  } else {
    const QString short_description = error.value(kShortDescriptionKey, "").toString();
    const QString full_description = error.value(kFullDescriptionKey, "").toString();

    ShowErrorMessage(short_description + "\n\n" + full_description);
  }
}

void DeviceViewModel::ShowErrorMessage(const QString& message) const {
  state_->SetErrorMessage(message);
  state_->SetShowErrorMessage(true);
}

void DeviceViewModel::HideErrorMessage() const {
  state_->SetShowErrorMessage(false);
}

void DeviceViewModel::SetErrorCodes(ErrorCodes *error_codes) {
  error_codes_ = error_codes;
  emit errorCodesChanged();
}

ErrorCodes* DeviceViewModel::GetErrorCodes() const {
  return error_codes_;
}

void DeviceViewModel::SetDeviceController(DeviceController *device_controller) {
  device_controller_ = device_controller;

  connect(
    device_controller_,
    &DeviceController::errorOccurred,
    [this](const QString& error_message) {
      ShowErrorMessageForDevice(error_message);
    });

  emit deviceControllerChanged();
}

DeviceController* DeviceViewModel::GetDeviceController() const {
  return device_controller_;
}
