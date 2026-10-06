#include "DeviceService.h"

DeviceService::DeviceService(IHomeApi* api, QObject* parent) : QObject(parent), api_(api) {}

void DeviceService::GetDeviceInfo(const QString& device_id, QObject* context,
                                ApiResultHandler<DeviceInfo> handler) {
  api_->GetDeviceInfo(device_id, context, std::move(handler));
}

void DeviceService::UseCapability(const QString& device_id, CapabilityType type,
                                const QVariantMap& state, QObject* context,
                                ApiResultHandler<void> handler) {
  const CapabilityObject action{.type = type, .state = state};
  api_->PerformActions({DeviceActionsObject{.id = device_id, .actions = {action}}},
                       context, std::move(handler));
}
