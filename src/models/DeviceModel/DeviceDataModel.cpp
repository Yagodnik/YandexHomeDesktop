#include "DeviceDataModel.h"
#include "DeviceController.h"

DeviceDataModel::DeviceDataModel(QObject* parent) : QObject(parent) {}

DeviceDataModel::DeviceDataModel(DeviceController* controller, QObject* parent)
  : DeviceDataModel(parent) {
  connect(controller, &DeviceController::loadRequestMade, this, &DeviceDataModel::ResetModel);
  connect(controller, &DeviceController::deviceDataReady, this, &DeviceDataModel::OnDeviceInfoReceived);
  connect(controller, &DeviceController::errorOccurred, this, &DeviceDataModel::OnDeviceInfoReceivingFailed);
}

QString DeviceDataModel::GetDeviceName() const {
  return device_name_;
}

bool DeviceDataModel::IsDeviceOnline() const {
  return device_state_ == DeviceState::Online;
}

void DeviceDataModel::ResetModel() {
  is_initialized_ = false;
  device_state_ = DeviceState::Offline;
  device_name_.clear();
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
    emit initializeFailed();

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

  if (!is_initialized_) {
    emit initializeFailed();
  }

  qCritical() << "DeviceDataModel: Cant receive device info due to:";
  qCritical() << message;
}
