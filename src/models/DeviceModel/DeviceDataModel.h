#pragma once

#include <QObject>

#include "services/DeviceSession.h"

class DeviceController;

class DeviceDataModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ GetDeviceName NOTIFY deviceNameChanged)
  Q_PROPERTY(bool isOnline READ IsDeviceOnline NOTIFY deviceStateChanged)
public:
  explicit DeviceDataModel(QObject* parent = nullptr);
  explicit DeviceDataModel(DeviceController* controller, QObject* parent = nullptr);
  void ResetModel();
  void OnDeviceInfoReceived(const DeviceInfo& info);
  void OnDeviceInfoReceivingFailed(const QString& message);

  [[nodiscard]] QString GetDeviceName() const;
  [[nodiscard]] bool IsDeviceOnline() const;

signals:
  void deviceNameChanged();
  void deviceStateChanged();
  void initialized();
  void initializeFailed();

private:
  void SetDeviceName(const QString &name);
  void SetDeviceStatus(DeviceState state);

  QString device_name_;
  DeviceState device_state_ = DeviceState::Offline;

  bool is_initialized_ = false;

};
