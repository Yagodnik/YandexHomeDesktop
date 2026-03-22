#include "DeviceViewState.h"

DeviceViewState::DeviceViewState(QObject *parent) : QObject(parent) {
}

bool DeviceViewState::GetShowErrorMessage() const {
  return show_error_message_;
}

void DeviceViewState::SetShowErrorMessage(const bool show_error_message) {
  show_error_message_ = show_error_message;
  emit showErrorMessageChanged();
}

const QString& DeviceViewState::GetErrorMessage() const {
  return error_message_;
}

void DeviceViewState::SetErrorMessage(const QString &error_message) {
  error_message_ = error_message;
  emit errorMessageChanged();
}

int DeviceViewState::GetCurrentPage() const {
  return current_page_;
}

void DeviceViewState::SetCurrentPage(const int current_page) {
  current_page_ = current_page;
  emit currentPageChanged();
}

bool DeviceViewState::HasCapabilities() const {
  return has_capabilities_;
}

void DeviceViewState::SetHasCapabilities(const bool has_capabilities) {
  has_capabilities_ = has_capabilities;
  emit hasCapabilitiesChanged();
}

bool DeviceViewState::HasProperties() const {
  return has_properties_;
}

void DeviceViewState::SetHasProperties(const bool has_properties) {
  has_properties_ = has_properties;
  emit hasPropertiesChanged();
}

bool DeviceViewState::IsDeviceOnline() const {
  return device_online_;
}

void DeviceViewState::SetDeviceOnline(const bool device_online) {
  device_online_ = device_online;
  emit deviceOnlineChanged();
}
