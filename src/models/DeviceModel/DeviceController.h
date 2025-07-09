#pragma once

#include <QTimer>

#include "DeviceAttribute.h"
#include "api/YandexHomeApi.h"

class DeviceController : public QObject {
  Q_OBJECT
public:
  explicit DeviceController(YandexHomeApi *api, QObject* parent = nullptr);

  Q_INVOKABLE void LoadDevice(const QString& device_id);

  void UseCapability(int index, const CapabilityObject& capability, const QVariantMap& state);

signals:
  void loadRequestMade();
  void capabilitiesUpdateReady(const QVariantList& capabilities);
  void propertiesUpdateReady(const QVariantList& properties);
  void capabilityUsed(int index, const QVariantMap& state);
  void errorOccurred(const QString& error_message);

private:
  static constexpr int kPollingInterval = 3000;

  QString device_id_;
  QList<DeviceAttribute> capabilities_updates_;
  double last_update_start_time_;

  QTimer polling_timer_;
  YandexHomeApi* api_;

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


