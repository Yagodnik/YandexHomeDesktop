#pragma once

#include <QTimer>
#include <QDateTime>
#include <functional>
#include <optional>

#include "DeviceAttribute.h"
#include "api/IHomeApi.h"

class DeviceController : public QObject {
  Q_OBJECT
public:
  using CapabilitiesList = QList<std::optional<CapabilityObject>>;
  using PropertiesList = QList<std::optional<PropertyObject>>;
  using TimeProvider = std::function<double()>;

  // Tests can advance the suppression window without waiting on wall-clock time.
  explicit DeviceController(IHomeApi *api, QObject* parent = nullptr,
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
  static constexpr int kPollingInterval = 3000;

  QString device_id_;
  bool is_in_use_ = false;
  QList<DeviceAttribute> capabilities_updates_;
  double last_update_start_time_;

  QTimer polling_timer_;
  IHomeApi* api_;
  TimeProvider time_provider_;

  static double CurrentTime() {
    return static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
  }

private slots:
  void OnTimerTimeout();
  void OnDeviceInfoReceived(const DeviceInfo& info);
  void OnDeviceInfoReceivingFailed(const QString& message);

  void OnActionExecutionFinishedSuccessfully(const QVariant& user_data);
  void OnActionExecutionFailed(const QString& message, const QVariant& user_data);
};
