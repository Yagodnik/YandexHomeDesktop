#pragma once

#include "CapabilitiesModel.h"
#include "PropertiesModel.h"
#include "DeviceDataModel.h"

class DeviceViewModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(PageState state READ GetState NOTIFY stateChanged)
  Q_PROPERTY(bool isLoading READ IsLoading NOTIFY stateChanged)
  Q_PROPERTY(QString deviceId READ GetDeviceId NOTIFY deviceIdChanged)
  Q_PROPERTY(CapabilitiesModel* capabilities READ GetCapabilities CONSTANT)
  Q_PROPERTY(PropertiesModel* properties READ GetProperties CONSTANT)
  Q_PROPERTY(DeviceDataModel* deviceData READ GetDeviceData CONSTANT)
  Q_PROPERTY(QUrl videoStreamUrl READ GetVideoStreamUrl NOTIFY videoStreamChanged)
  Q_PROPERTY(bool videoStreamLoading READ IsVideoStreamLoading NOTIFY videoStreamChanged)
  Q_PROPERTY(bool videoStreamFailed READ HasVideoStreamFailed NOTIFY videoStreamChanged)
public:
  enum PageState { Loading, Error, Ready };
  Q_ENUM(PageState)
  using CapabilitiesList = DeviceSession::CapabilitiesList;
  using PropertiesList = DeviceSession::PropertiesList;
  using TimeProvider = DeviceSession::TimeProvider;

  explicit DeviceViewModel(DeviceService* service, QObject* parent = nullptr,
                           TimeProvider time_provider = {});

  [[nodiscard]] PageState GetState() const;
  [[nodiscard]] bool IsLoading() const;
  [[nodiscard]] QString GetDeviceId() const;
  CapabilitiesModel* GetCapabilities();
  PropertiesModel* GetProperties();
  DeviceDataModel* GetDeviceData();
  QUrl GetVideoStreamUrl() const {
    return session_.VideoStreamUrl();
  }
  bool IsVideoStreamLoading() const {
    return session_.VideoStreamLoading();
  }
  bool HasVideoStreamFailed() const {
    return session_.VideoStreamFailed();
  }
  Q_INVOKABLE void RequestVideoStream() {
    session_.RequestVideoStream();
  }
  Q_INVOKABLE void StopVideoStream() {
    session_.StopVideoStream();
  }

  Q_INVOKABLE void LoadDevice(const QString& device_id);
  Q_INVOKABLE void TryReloadDevice();
  Q_INVOKABLE void ContinuePollingIfNeeded();
  Q_INVOKABLE void StopPolling();
  Q_INVOKABLE void ForgetDevice();
  void ResetSession();
  void Refresh();

  // Compatibility signals for consumers of the former deviceController context.
signals:
  void videoStreamChanged();
  void stateChanged();
  void deviceIdChanged();
  void loadRequestMade();
  void capabilitiesUpdateReady(const CapabilitiesList& capabilities);
  void propertiesUpdateReady(const PropertiesList& properties);
  void deviceDataReady(const DeviceInfo& info);
  void capabilityUsed(int index, const QVariantMap& state);
  void errorOccurred(const QString& error_message);
  void deviceInfoReceived(const DeviceInfo& info);
  void deviceInfoReceivingFailed(const QString& message);

private:
  void Reset();
  void UpdateReadiness();
  void SetState(PageState state);

  DeviceSession session_;
  CapabilitiesModel capabilities_;
  PropertiesModel properties_;
  DeviceDataModel device_data_;
  PageState state_ = Loading;
  bool capabilities_ready_ = false;
  bool properties_ready_ = false;
  bool device_data_ready_ = false;
  QString device_id_;
};
