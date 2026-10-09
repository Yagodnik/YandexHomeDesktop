#include "DeviceService.h"

#include <QPointer>

#include "iot/core/CapabilityRules.h"

DeviceService::DeviceService(IHomeApi* api, QObject* parent) : QObject(parent), api_(api) {}

void DeviceService::GetDeviceInfo(
  const QString& device_id, QObject* context, ApiResultHandler<DeviceInfo> handler) {
  api_->GetDeviceInfo(device_id, context, std::move(handler));
}

void DeviceService::UseCapability(const QString& device_id, CapabilityType type,
  const QVariantMap& state, QObject* context, ApiResultHandler<void> handler) {
  const CapabilityObject action{.type = type, .state = state};
  api_->PerformActions(
    {DeviceActionsObject{.id = device_id, .actions = {action}}}, context, std::move(handler));
}

void DeviceService::ApplyCapability(const QString& device_id, CapabilityType type,
  const QVariantMap& state, QObject* context, CommandResultHandler handler) {
  const auto rules = Iot::Rules::ForType(type);
  if (!rules) {
    handler(std::unexpected(CommandRejection{CommandRejectionReason::CapabilityNotFound, {}}));
    return;
  }
  const auto input = Iot::ValidateInput(*rules, state);
  if (!input) {
    handler(std::unexpected(CommandRejection{CommandRejectionReason::InvalidValue, input.error()}));
    return;
  }

  const QPointer<DeviceService> service(this);
  GetDeviceInfo(device_id, context,
    [service, device_id, rules, state, context, handler = std::move(handler)](
      ApiResult<DeviceInfo> result) mutable {
      if (!service) {
        return;
      }
      if (!result) {
        handler(std::unexpected(result.error()));
        return;
      }
      if (result->status != Status::Ok || result->id != device_id) {
        handler(
          std::unexpected(CommandRejection{CommandRejectionReason::InvalidDeviceResponse, {}}));
        return;
      }

      const CapabilityObject* match = nullptr;
      for (const auto& capability : result->capabilities) {
        if (!rules->Matches(capability, state)) {
          continue;
        }
        if (match) {
          handler(
            std::unexpected(CommandRejection{CommandRejectionReason::AmbiguousCapability, {}}));
          return;
        }
        match = &capability;
      }
      if (!match) {
        handler(std::unexpected(CommandRejection{CommandRejectionReason::CapabilityNotFound, {}}));
        return;
      }
      const auto valid = rules->ValidateDevice(match->parameters, state);
      if (!valid) {
        handler(
          std::unexpected(CommandRejection{CommandRejectionReason::InvalidValue, valid.error()}));
        return;
      }

      service->UseCapability(device_id, rules->Type(), state, context,
        [handler = std::move(handler)](ApiResult<void> result) {
          if (!result) {
            handler(std::unexpected(result.error()));
            return;
          }
          handler(CommandResult{});
        });
    });
}
