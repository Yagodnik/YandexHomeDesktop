#pragma once

#include <QTimer>

#include "CapabilitiesModel.h"
#include "PropertiesModel.h"
#include "api/YandexHomeApi.h"

class DeviceDataController : public QObject {
  Q_OBJECT
public:
  explicit DeviceDataController(YandexHomeApi *api, QObject* parent = nullptr);

  using CapabilitiesList = QList<std::optional<CapabilityObject>>;

  Q_INVOKABLE void RequestData(const QString& device_id);
  Q_INVOKABLE void EnablePolling();
  Q_INVOKABLE void DisablePolling();

  void UseCapability(int index, const CapabilityObject& capability, const QVariantMap& state) const;

signals:
  void capabilitiesUpdateReady(const QVariantList& capabilities);
  void propertiesUpdateReady(const QVariantList& properties);
  void capabilityUsed(int index, const QVariantMap& state);

private:
  static constexpr int kPollingInterval = 3000;

  double last_update_start_time_;

  QTimer timer_;

  QString device_id_;
  CapabilitiesModel *capabilities_model_;
  PropertiesModel *properties_model_;

  YandexHomeApi* api_;

  static double CurrentTime() {
    return static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
  }

private slots:
  void OnTimerTimeout();
  void OnDeviceInfoReceived(const DeviceObject& info);
  void OnDeviceInfoReceivingFailed(const QString& message);
};


