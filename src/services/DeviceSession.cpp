#include "DeviceSession.h"

#include <QDateTime>
#include <ranges>

namespace {
double CurrentTime() {
  return static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
}
}

DeviceSession::DeviceSession(DeviceService* service, QObject* parent, TimeProvider time_provider)
  : QObject(parent), service_(service),
    time_provider_(time_provider ? std::move(time_provider) : TimeProvider{CurrentTime}) {
  polling_timer_.setInterval(3000);
  polling_timer_.setSingleShot(false);
  connect(&polling_timer_, &QTimer::timeout, this, &DeviceSession::Refresh);
}

bool DeviceSession::CapabilityUpdate::Contains(double time) const {
  constexpr double ignore_delta = 0.8;
  return time >= start_time - ignore_delta && time < finish_time + ignore_delta;
}

void DeviceSession::LoadDevice(const QString& device_id) {
  device_id_ = device_id;
  last_update_start_time_ = time_provider_();
  capabilities_updates_.clear();
  is_in_use_ = true;
  // Models must reset before an API implementation can reply synchronously.
  emit loadRequestMade();
  service_->GetDeviceInfo(device_id_, this,
    [this](ApiResult<DeviceInfo> result) { ReceiveDeviceInfo(std::move(result)); });
}

void DeviceSession::TryReloadDevice() { LoadDevice(device_id_); }
void DeviceSession::ContinuePollingIfNeeded() {
  if (is_in_use_) {
    polling_timer_.start();
  }
}
void DeviceSession::StopPolling() { polling_timer_.stop(); }
bool DeviceSession::IsPolling() const { return polling_timer_.isActive(); }
void DeviceSession::ForgetDevice() {
  StopPolling();
  device_id_.clear();
  is_in_use_ = false;
}

void DeviceSession::Refresh() {
  last_update_start_time_ = time_provider_();
  service_->GetDeviceInfo(device_id_, this,
    [this](ApiResult<DeviceInfo> result) { ReceiveDeviceInfo(std::move(result)); });
}

void DeviceSession::UseCapability(int index, CapabilityType type, const QVariantMap& state) {
  if (index >= 0 && index < capabilities_updates_.size()) {
    auto& update = capabilities_updates_[index];
    update.pending = true;
    update.start_time = time_provider_();
  }
  service_->UseCapability(device_id_, type, state, this,
    [this, index](ApiResult<void> result) { FinishAction(index, std::move(result)); });
}

void DeviceSession::ReceiveDeviceInfo(ApiResult<DeviceInfo> result) {
  if (!result) {
    emit deviceInfoReceivingFailed(result.error().message);
    emit errorOccurred(result.error().message);
    return;
  }
  emit deviceInfoReceived(*result);
  ApplyDeviceInfo(*result);
}

void DeviceSession::ApplyDeviceInfo(const DeviceInfo& info) {
  if (info.id != device_id_) {
    return;
  }
  emit deviceDataReady(info);

  const double receive_time = time_provider_();
  if (capabilities_updates_.empty()) {
    capabilities_updates_.resize(info.capabilities.size());
  }

  CapabilitiesList capabilities;
  for (const auto& [update, incoming] : std::views::zip(capabilities_updates_, info.capabilities)) {
    if (update.pending || update.Contains(receive_time) || update.Contains(last_update_start_time_)) {
      capabilities.push_back(std::nullopt);
    } else {
      capabilities.push_back(incoming);
    }
  }
  PropertiesList properties;
  for (const auto& property : info.properties) {
    properties.push_back(property);
  }

  emit capabilitiesUpdateReady(capabilities);
  emit propertiesUpdateReady(properties);
  // Preserve the existing behavior: even a reply after StopPolling restarts it.
  polling_timer_.start();
}

void DeviceSession::FinishAction(int index, ApiResult<void> result) {
  // Compatibility: either completion clears the shared row flag, including a
  // completion from before a selection change. Correlation is a separate change.
  if (index >= 0 && index < capabilities_updates_.size()) {
    auto& update = capabilities_updates_[index];
    update.pending = false;
    update.finish_time = time_provider_();
  }
  if (!result) {
    emit errorOccurred(result.error().message);
  }
}
