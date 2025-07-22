#include "DeviceDataModel.h"

DeviceDataModel::DeviceDataModel(YandexHomeApi *api, QObject *parent) :
  QObject(parent),
  api_(api),
  device_state_(DeviceState::Offline),
  is_initialized_(false)
{
  connect(api_,
    &YandexHomeApi::deviceInfoReceived,
    this,
    &DeviceDataModel::OnDeviceInfoReceived);

  connect(api_,
    &YandexHomeApi::deviceInfoReceivingFailed,
    this,
    &DeviceDataModel::OnDeviceInfoReceivingFailed);
}

QString DeviceDataModel::GetDeviceName() const {
  return device_name_;
}

bool DeviceDataModel::IsDeviceOnline() const {
  return device_state_ == DeviceState::Online;
}

void DeviceDataModel::SetDeviceName(const QString &name) {
  device_name_ = name;
  emit deviceNameChanged();
}

void DeviceDataModel::SetDeviceStatus(DeviceState state) {
  device_state_ = state;
  emit deviceStateChanged();
}

void DeviceDataModel::OnDeviceInfoReceived(const DeviceInfo &info) {
  if (info.status != Status::Ok) {
    qCritical() << "DeviceDataModel: Failed to receive data due to";
    qCritical() << info.message;
    return;
  }

  SetDeviceName(info.name);
  SetDeviceStatus(info.state);

  qInfo() << "DeviceDataModel: update for" << device_name_
    << " status:" << (device_state_ == DeviceState::Online ? "Online" : "Offline");

  if (!is_initialized_) {
    emit initialized();
  }
  is_initialized_ = true;
}

void DeviceDataModel::OnDeviceInfoReceivingFailed(const QString &message) {
  device_state_ = DeviceState::Offline;

  qCritical() << "DeviceDataModel: Cant receive device info due to:";
  qCritical() << message;
}
