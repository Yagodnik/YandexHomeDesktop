#pragma once

#include <QObject>

#include "DeviceController.h"
#include "api/YandexHomeApi.h"

class DeviceDataModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ GetDeviceName NOTIFY deviceNameChanged)
  Q_PROPERTY(bool isOnline READ IsDeviceOnline NOTIFY deviceStateChanged)
public:
  explicit DeviceDataModel(DeviceController *controller, QObject *parent = nullptr);

  [[nodiscard]] QString GetDeviceName() const;
  [[nodiscard]] bool IsDeviceOnline() const;

signals:
  void deviceNameChanged();
  void deviceStateChanged();
  void initialized();
  void initializeFailed();

private:
  void ResetModel();

  void SetDeviceName(const QString &name);
  void SetDeviceStatus(DeviceState state);

  DeviceController *controller_;

  QString device_name_;
  DeviceState device_state_;

  bool is_initialized_;

private slots:
  void OnDeviceInfoReceived(const DeviceInfo& info);
  void OnDeviceInfoReceivingFailed(const QString& message);
};
