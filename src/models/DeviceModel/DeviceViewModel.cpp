#include "DeviceViewModel.h"

DeviceViewModel::DeviceViewModel(DeviceService* service, QObject* parent, TimeProvider time_provider)
  : QObject(parent), session_(service, this, std::move(time_provider)),
    capabilities_(this), properties_(this), device_data_(this) {
  connect(&session_, &DeviceSession::loadRequestMade, this, &DeviceViewModel::Reset);
  connect(&session_, &DeviceSession::deviceDataReady, this, [this](const DeviceInfo& info) {
    emit deviceDataReady(info);
    device_data_.OnDeviceInfoReceived(info);
  });
  connect(&session_, &DeviceSession::capabilitiesUpdateReady, this, [this](const CapabilitiesList& update) {
    emit capabilitiesUpdateReady(update);
    capabilities_.OnCapabilitiesUpdated(update);
  });
  connect(&session_, &DeviceSession::propertiesUpdateReady, this, [this](const PropertiesList& update) {
    emit propertiesUpdateReady(update);
    properties_.OnPropertiesUpdateReady(update);
  });
  connect(&session_, &DeviceSession::errorOccurred, this, [this](const QString& message) {
    properties_.OnPropertiesUpdateFailed(message);
    device_data_.OnDeviceInfoReceivingFailed(message);
    emit errorOccurred(message);
  });
  connect(&session_, &DeviceSession::deviceInfoReceived, this, &DeviceViewModel::deviceInfoReceived);
  connect(&session_, &DeviceSession::deviceInfoReceivingFailed, this, &DeviceViewModel::deviceInfoReceivingFailed);
  connect(&capabilities_, &CapabilitiesModel::capabilityRequested, this,
    [this](int index, const CapabilityObject& capability, const QVariantMap& state) {
      session_.UseCapability(index, capability.type, state);
    });

  connect(&capabilities_, &CapabilitiesModel::initialized, this, [this] {
    capabilities_ready_ = true;
    UpdateReadiness();
  });
  connect(&properties_, &PropertiesModel::initialized, this, [this] {
    properties_ready_ = true;
    UpdateReadiness();
  });
  connect(&device_data_, &DeviceDataModel::initialized, this, [this] {
    device_data_ready_ = true;
    UpdateReadiness();
  });
  connect(&capabilities_, &CapabilitiesModel::initializeFailed, this, [this] { SetState(Error); });
  connect(&properties_, &PropertiesModel::initializeFailed, this, [this] { SetState(Error); });
  connect(&device_data_, &DeviceDataModel::initializeFailed, this, [this] { SetState(Error); });
}

DeviceViewModel::PageState DeviceViewModel::GetState() const { return state_; }
bool DeviceViewModel::IsLoading() const { return state_ == Loading; }
CapabilitiesModel* DeviceViewModel::GetCapabilities() { return &capabilities_; }
PropertiesModel* DeviceViewModel::GetProperties() { return &properties_; }
DeviceDataModel* DeviceViewModel::GetDeviceData() { return &device_data_; }
void DeviceViewModel::LoadDevice(const QString& device_id) { session_.LoadDevice(device_id); }
void DeviceViewModel::TryReloadDevice() { session_.TryReloadDevice(); }
void DeviceViewModel::ContinuePollingIfNeeded() { session_.ContinuePollingIfNeeded(); }
void DeviceViewModel::StopPolling() { session_.StopPolling(); }
void DeviceViewModel::ForgetDevice() { session_.ForgetDevice(); }
void DeviceViewModel::ResetSession() { session_.ResetSession(); }
void DeviceViewModel::Refresh() { session_.Refresh(); }

void DeviceViewModel::Reset() {
  capabilities_ready_ = properties_ready_ = device_data_ready_ = false;
  capabilities_.ResetModel();
  properties_.ResetModel();
  device_data_.ResetModel();
  SetState(Loading);
  emit loadRequestMade();
}

void DeviceViewModel::UpdateReadiness() {
  if (capabilities_ready_ && properties_ready_ && device_data_ready_) {
    SetState(Ready);
  }
}

void DeviceViewModel::SetState(PageState state) {
  if (state_ != state) {
    state_ = state;
    emit stateChanged();
  }
}
