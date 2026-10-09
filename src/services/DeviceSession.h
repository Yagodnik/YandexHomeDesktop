#pragma once

#include <QTimer>
#include <functional>
#include <optional>
#include "DeviceService.h"

// One consumer's observation of a device. Separate sessions can share a service
// without sharing their selection, timers, or capability suppression windows.
class DeviceSession final : public QObject {
  Q_OBJECT
public:
  using CapabilitiesList = QList<std::optional<CapabilityObject>>;
  using PropertiesList = QList<std::optional<PropertyObject>>;
  using TimeProvider = std::function<double()>;

  explicit DeviceSession(DeviceService* service, QObject* parent = nullptr,
                         TimeProvider time_provider = {});

  void LoadDevice(const QString& device_id);
  void TryReloadDevice();
  void ContinuePollingIfNeeded();
  void StopPolling();
  void ForgetDevice();
  void ResetSession();
  [[nodiscard]] bool IsPolling() const;
  void Refresh();
  void UseCapability(int index, CapabilityType type, const QVariantMap& state);

signals:
  void loadRequestMade();
  void capabilitiesUpdateReady(const CapabilitiesList& capabilities);
  void propertiesUpdateReady(const PropertiesList& properties);
  void deviceDataReady(const DeviceInfo& info);
  void errorOccurred(const QString& error_message);
  void deviceInfoReceived(const DeviceInfo& info);
  void deviceInfoReceivingFailed(const QString& message);

private:
  struct CapabilityUpdate {
    bool pending = false;
    double start_time = 0;
    double finish_time = 0;
    [[nodiscard]] bool Contains(double time) const;
  };

  void ReceiveDeviceInfo(ApiResult<DeviceInfo> result);
  void ApplyDeviceInfo(const DeviceInfo& info);
  void FinishAction(int index, ApiResult<void> result);

  DeviceService* service_;
  TimeProvider time_provider_;
  QTimer polling_timer_;
  QString device_id_;
  bool is_in_use_ = false;
  quint64 session_generation_ = 0;
  QList<CapabilityUpdate> capabilities_updates_;
  // Intentionally shared across reads, matching the characterized controller.
  double last_update_start_time_ = 0;
};
