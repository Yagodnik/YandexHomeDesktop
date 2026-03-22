#pragma once

#include "DeviceViewState.h"
#include "models/DeviceModel/DeviceController.h"
#include "utils/ErrorCodes.h"

class DeviceViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(DeviceViewState* state READ GetState CONSTANT)
  Q_PROPERTY(ErrorCodes* errorCodesDep READ GetErrorCodes WRITE SetErrorCodes NOTIFY errorCodesChanged REQUIRED)
  Q_PROPERTY(DeviceController* deviceControllerDep READ GetDeviceController WRITE SetDeviceController NOTIFY deviceControllerChanged REQUIRED)
public:
  explicit DeviceViewModel(QObject* parent = nullptr);

  [[nodiscard]] DeviceViewState* GetState() const;

  Q_INVOKABLE void ShowErrorMessageForDevice(const QString& error_code) const;
  Q_INVOKABLE void ShowErrorMessage(const QString& message) const;
  Q_INVOKABLE void HideErrorMessage() const;

  void SetErrorCodes(ErrorCodes* error_codes);
  [[nodiscard]] ErrorCodes* GetErrorCodes() const;

  void SetDeviceController(DeviceController* device_controller);
  [[nodiscard]] DeviceController* GetDeviceController() const;

signals:
  void errorCodesChanged();
  void deviceControllerChanged();

private:
  const char* kErrorOccurredMessage = "Произошла ошибка!";
  const QString kShortDescriptionKey = "short_description";
  const QString kFullDescriptionKey = "full_description";

  std::unique_ptr<DeviceViewState> state_;

  ErrorCodes* error_codes_{};
  DeviceController* device_controller_{};
};
