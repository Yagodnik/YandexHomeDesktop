#pragma once

#include "services/DeviceSession.h"

// Compatibility adapter for existing C++ consumers. The desktop now uses
// DeviceViewModel; polling and reconciliation live entirely in DeviceSession.
class DeviceController : public QObject {
  Q_OBJECT
public:
  using CapabilitiesList = DeviceSession::CapabilitiesList;
  using PropertiesList = DeviceSession::PropertiesList;
  using TimeProvider = DeviceSession::TimeProvider;

  explicit DeviceController(IHomeApi* api, QObject* parent = nullptr,
                            TimeProvider time_provider = {});

  Q_INVOKABLE void LoadDevice(const QString& device_id);
  Q_INVOKABLE void TryReloadDevice();
  Q_INVOKABLE void ContinuePollingIfNeeded();
  Q_INVOKABLE void StopPolling();
  Q_INVOKABLE void ForgetDevice();
  void UseCapability(int index, const CapabilityObject& capability, const QVariantMap& state);

signals:
  void loadRequestMade();
  void capabilitiesUpdateReady(const CapabilitiesList& capabilities);
  void propertiesUpdateReady(const PropertiesList& properties);
  void deviceDataReady(const DeviceInfo& info);
  void capabilityUsed(int index, const QVariantMap& state);
  void errorOccurred(const QString& error_message);
  void deviceInfoReceived(const DeviceInfo& info);
  void deviceInfoReceivingFailed(const QString& message);

private:
  DeviceService service_;
  DeviceSession session_;

private slots:
  void OnTimerTimeout();
};
