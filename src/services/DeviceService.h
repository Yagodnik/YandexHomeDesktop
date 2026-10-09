#pragma once

#include "yh/appservices_export.h"

#include <QObject>
#include "api/IHomeApi.h"
#include "CommandResult.h"

// Stateless device operations. Callers supply an explicit device ID and own
// callback delivery; no GUI selection or polling is required to send a command.
class APPSERVICES_EXPORT DeviceService final : public QObject {
  Q_OBJECT
public:
  explicit DeviceService(IHomeApi* api, QObject* parent = nullptr);

  void GetDeviceInfo(
    const QString& device_id, QObject* context, ApiResultHandler<DeviceInfo> handler);
  void UseCapability(const QString& device_id, CapabilityType type, const QVariantMap& state,
    QObject* context, ApiResultHandler<void> handler);
  // Validate a one-shot command against fresh device metadata before sending it.
  void ApplyCapability(const QString& device_id, CapabilityType type, const QVariantMap& state,
    QObject* context, CommandResultHandler handler);

private:
  IHomeApi* api_;
};
