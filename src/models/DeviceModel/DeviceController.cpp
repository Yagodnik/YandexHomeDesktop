#include "DeviceController.h"
#include <QDebug>
#include <ranges>

DeviceController::DeviceController(IHomeApi *api, QObject *parent, TimeProvider time_provider)
  : QObject(parent), api_(api),
    time_provider_(time_provider ? std::move(time_provider) : TimeProvider{CurrentTime})
{
  connect(&polling_timer_,
    &QTimer::timeout,
    this,
    &DeviceController::OnTimerTimeout);

  polling_timer_.setInterval(kPollingInterval);
  polling_timer_.setSingleShot(false);
}

void DeviceController::LoadDevice(const QString &device_id) {
  device_id_ = device_id;

  qDebug() << "Device Controller: Load device" << device_id;

  last_update_start_time_ = time_provider_();
  capabilities_updates_.clear();
  is_in_use_ = true;

  emit loadRequestMade();

  api_->GetDeviceInfo(device_id_, this, [this](ApiResult<DeviceInfo> result) {
    if (result) {
      emit deviceInfoReceived(*result);
      OnDeviceInfoReceived(*result);
    } else {
      emit deviceInfoReceivingFailed(result.error().message);
      OnDeviceInfoReceivingFailed(result.error().message);
    }
  });

}

void DeviceController::TryReloadDevice() {
  LoadDevice(device_id_);
}

void DeviceController::ContinuePollingIfNeeded() {
  if (is_in_use_) {
    qInfo() << "Device Controller: Continued polling for device" << device_id_;
    polling_timer_.start();
  }
}

void DeviceController::StopPolling() {
  if (polling_timer_.isActive()) {
    qInfo() << "Device Controller: Stopped polling for device" << device_id_;
    polling_timer_.stop();
  }
}

void DeviceController::ForgetDevice() {
  StopPolling();
  qInfo() << "Device Controller: Forgetting device" << device_id_;

  device_id_ = "";
  is_in_use_ = false;
}

void DeviceController::UseCapability(
  const int index,
  const CapabilityObject &capability,
  const QVariantMap &state
) {
  const CapabilityObject action = {
    .type = capability.type,
    .state = state
  };

  const DeviceActionsObject action_object = {
    .id = device_id_,
    .actions = { action }
  };

  if (index < capabilities_updates_.size()) {
    capabilities_updates_[index].PausePolling(time_provider_());
  } else {
    qInfo() << "Device Controller: Cant pause polling for" << index << "as it out of range:" << capabilities_updates_.size();
  }

  api_->PerformActions({action_object}, this, [this, index](ApiResult<void> result) {
    if (result) {
      OnActionExecutionFinishedSuccessfully(index);
    } else {
      OnActionExecutionFailed(result.error().message, index);
    }
  });
}

void DeviceController::OnTimerTimeout() {
  qInfo() << "Device Controller: Polling tick!";

  last_update_start_time_ = time_provider_();
  api_->GetDeviceInfo(device_id_, this, [this](ApiResult<DeviceInfo> result) {
    if (result) {
      emit deviceInfoReceived(*result);
      OnDeviceInfoReceived(*result);
    } else {
      emit deviceInfoReceivingFailed(result.error().message);
      OnDeviceInfoReceivingFailed(result.error().message);
    }
  });

  qInfo() << "Device Controller: Update requested";
}

void DeviceController::OnDeviceInfoReceived(const DeviceInfo& info) {
  if (info.id != device_id_) {
    qInfo() << "Device Controller: Got update for wrong device, skipping";
    qInfo() << "Device Controller: Expected:" << device_id_ << "Got:" << info.id;

    return;
  }

  emit deviceDataReady(info);

  const double receive_time = time_provider_();
  constexpr double ignore_delta = 0.8;

  qInfo() << "Device Controller: received device info:" << info.name;
  qInfo() << "Number of caps:" << info.capabilities.size() <<
              "| Number of props:" << info.properties.size();

  if (capabilities_updates_.empty()) {
    qInfo() << "Device Contoller: Filling updates array with" << info.capabilities.size() << "items";
    capabilities_updates_.resize(info.capabilities.size());
  }

  CapabilitiesList capabilities;
  PropertiesList properties;

  const auto blocking_condition = [this, receive_time](const auto& attr) -> bool {
    return attr.is_pending ||
           attr.IsInsideInterval(receive_time, ignore_delta) ||
           attr.IsInsideInterval(last_update_start_time_, ignore_delta);
  };

  for (auto [attr, incoming] : std::ranges::views::zip(capabilities_updates_, info.capabilities)) {
    if (blocking_condition(attr)) {
      capabilities.push_back(std::nullopt);
    } else {
      capabilities.push_back(incoming);
    }
  }

  for (const auto& property : info.properties) {
    properties.push_back(property);
  }

  emit capabilitiesUpdateReady(capabilities);
  emit propertiesUpdateReady(properties);

  polling_timer_.start();
}

void DeviceController::OnDeviceInfoReceivingFailed(const QString &message) {
  qCritical() << "Device Controller: unable to receive device info!";
  qCritical() << "Device Controller: Message:" << message;

  emit errorOccurred(message);
}

void DeviceController::OnActionExecutionFinishedSuccessfully(const QVariant &user_data) {
  // TODO: Add blocking accessing data from previous device

  qInfo() << "Device Controller: Action executed successfully";

  const int index = user_data.toInt();

  if (index < capabilities_updates_.size()) {
    capabilities_updates_[index].ResumePolling(time_provider_());
  }
}

void DeviceController::OnActionExecutionFailed(const QString &message, const QVariant &user_data) {
  // TODO: Add blocking accessing data from previous device

  qCritical() << "Device Controller: Error executing action:" << message;

  const int index = user_data.toInt();

  if (index < capabilities_updates_.size()) {
    capabilities_updates_[index].ResumePolling(time_provider_());
  }

  emit errorOccurred(message);
}
