#pragma once

#include <QAbstractListModel>
#include "api/YandexHomeApi.h"
#include "DeviceAttribute.h"

class CapabilitiesModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit CapabilitiesModel(YandexHomeApi *api, QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    DelegateSourceRole,
    AttributeTypeRole,
    BusyRole,
    StateRole,
    ParametersRole
  };

  void ResetModel(const QString& device_id);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  void ResetModel();

  Q_INVOKABLE void RequestData(const QString& device_id);
  void RequestUpdate();

  Q_INVOKABLE [[nodiscard]] QVariantMap GetState(int index) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap GetParameters(int index) const;

  Q_INVOKABLE void UseCapability(int index, const QVariantMap& state);

signals:
  void dataLoaded();
  void dataLoadingFailed();
  void dataUpdated(int index);
  void errorOccurred(const QString& error_message);
  void initialized();

// private:
public:
  const QString kUnsupportedDelegate = "qrc:/controls/Unsupported.qml";
  const QMap<CapabilityType, QString> kDelegates = {
    { CapabilityType::OnOff,        "qrc:/controls/OnOff.qml" },
    { CapabilityType::VideoStream,  kUnsupportedDelegate },
    { CapabilityType::ColorSetting, "qrc:/controls/ColorSetting.qml" },
    { CapabilityType::Mode,         "qrc:/controls/Mode.qml" },
    { CapabilityType::Range,        "qrc:/controls/Range.qml" },
    { CapabilityType::Toggle,       "qrc:/controls/Toggle.qml" },
  };

  const QMap<QString, QString> kDelegates2 = {
    { "devices.capabilities.on_off",        "qrc:/controls/OnOff.qml" },
    // { "devices.capabilities.on_off",        "qrc:/controls/Mode.qml" },
    { "devices.capabilities.video_stream",  kUnsupportedDelegate },
    { "devices.capabilities.color_setting", "qrc:/controls/ColorSetting.qml" },
    { "devices.capabilities.mode",          "qrc:/controls/Mode.qml" },
    { "devices.capabilities.range",         "qrc:/controls/Range.qml" },
    { "devices.capabilities.toggle",        "qrc:/controls/Toggle.qml" },
  };

  QString device_id_;
  YandexHomeApi *api_;

  QList<DeviceAttribute> capabilities_;
  bool is_initialized_;

  double last_update_start_time_;

  QTimer timer_;

private slots:
  void OnDeviceInfoReceived(const DeviceInfo& info);
  void OnDeviceInfoReceivingFailed(const QString& message);

  void OnActionExecutionFinishedSuccessfully(const QVariant& user_data);
  void OnActionExecutionFailed(const QString& message, const QVariant& user_data);
};
