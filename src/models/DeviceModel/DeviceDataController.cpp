#include "DeviceDataController.h"

DeviceDataController::DeviceDataController(YandexHomeApi *api, QObject *parent)
  : QObject(parent), capabilities_model_(nullptr), properties_model_(nullptr), api_(api)
{
  connect(api_,
    &YandexHomeApi::deviceInfoReceived,
    this,
    &DeviceDataController::OnDeviceInfoReceived);

  connect(api_,
    &YandexHomeApi::deviceInfoReceived,
    this,
    &DeviceDataController::OnDeviceInfoReceived);

  connect(&timer_,
    &QTimer::timeout,
    this,
    &DeviceDataController::OnTimerTimeout);
}

void DeviceDataController::LoadDevice(const QString &device_id) {
  device_id_ = device_id;

  last_update_start_time_ = CurrentTime();
  api_->GetDeviceInfo(device_id_);

  emit loadRequestMade();
}

void DeviceDataController::EnablePolling() {

}

void DeviceDataController::DisablePolling() {

}

void DeviceDataController::UseCapability(
  const int index,
  const CapabilityObject &capability,
  const QVariantMap &state
) const {
  const CapabilityObject action = {
    .type = capability.type,
    .state = state
  };

  const DeviceActionsObject action_object = {
    .id = device_id_,
    .actions = { action }
  };

  api_->PerformActions({
    action_object
  }, index);
}

void DeviceDataController::OnTimerTimeout() {
  qDebug() << "Making polling request";

  api_->GetDeviceInfo(device_id_);
}

void DeviceDataController::OnDeviceInfoReceived(const DeviceObject &info) {
  const double receive_time = CurrentTime();
  constexpr double ignore_delta = 0.8;

  QVariantList capabilities;
  QVariantList properties;

  const auto blocking_condition = [this, receive_time](const auto& attr) -> bool {
    return attr.is_pending ||
           attr.IsInsideInterval(receive_time, ignore_delta) ||
           attr.IsInsideInterval(last_update_start_time_, ignore_delta);
  };

  for (const auto& capability : info.capabilities) {
    if (blocking_condition(capability)) {
      capabilities.push_back({});
    }

    capabilities.push_back(capability.state);
  }

  for (const auto& property : info.properties) {
    properties.push_back(property.state);
  }

  emit capabilitiesUpdateReady(std::move(capabilities));
  emit propertiesUpdateReady(std::move(properties));
}

void DeviceDataController::OnDeviceInfoReceivingFailed(const QString &message) {
  qDebug() << "Device Data Controller: unable to receive device info!";
  qDebug() << message;
}
