#pragma once

#include <QObject>
#include "api/IHomeApi.h"

// Stateless device operations. Callers supply an explicit device ID and own
// callback delivery; no GUI selection or polling is required to send a command.
class DeviceService final : public QObject {
  Q_OBJECT
public:
  explicit DeviceService(IHomeApi* api, QObject* parent = nullptr);

  void GetDeviceInfo(const QString& device_id, QObject* context,
                     ApiResultHandler<DeviceInfo> handler);
  void UseCapability(const QString& device_id, CapabilityType type,
                     const QVariantMap& state, QObject* context,
                     ApiResultHandler<void> handler);

private:
  IHomeApi* api_;
};
