#include "DeviceController.h"

DeviceController::DeviceController(IHomeApi* api, QObject* parent, TimeProvider time_provider)
  : QObject(parent), service_(api, this), session_(&service_, this, std::move(time_provider)) {
  connect(&session_, &DeviceSession::loadRequestMade, this, &DeviceController::loadRequestMade);
  connect(&session_, &DeviceSession::capabilitiesUpdateReady, this, &DeviceController::capabilitiesUpdateReady);
  connect(&session_, &DeviceSession::propertiesUpdateReady, this, &DeviceController::propertiesUpdateReady);
  connect(&session_, &DeviceSession::deviceDataReady, this, &DeviceController::deviceDataReady);
  connect(&session_, &DeviceSession::errorOccurred, this, &DeviceController::errorOccurred);
  connect(&session_, &DeviceSession::deviceInfoReceived, this, &DeviceController::deviceInfoReceived);
  connect(&session_, &DeviceSession::deviceInfoReceivingFailed, this, &DeviceController::deviceInfoReceivingFailed);
}

void DeviceController::LoadDevice(const QString& device_id) { session_.LoadDevice(device_id); }
void DeviceController::TryReloadDevice() { session_.TryReloadDevice(); }
void DeviceController::ContinuePollingIfNeeded() { session_.ContinuePollingIfNeeded(); }
void DeviceController::StopPolling() { session_.StopPolling(); }
void DeviceController::ForgetDevice() { session_.ForgetDevice(); }
void DeviceController::UseCapability(int index, const CapabilityObject& capability, const QVariantMap& state) {
  session_.UseCapability(index, capability.type, state);
}
void DeviceController::OnTimerTimeout() { session_.Refresh(); }
