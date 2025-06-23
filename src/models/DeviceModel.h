#pragma once

#include <QAbstractListModel>
#include "api/YandexHomeApi.h"

class DeviceModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit DeviceModel(YandexHomeApi *api, QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    LastUpdateTimeRole,
    DelegateSourceRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void SetId(const QString& device_id);
  Q_INVOKABLE [[nodiscard]] CapabilityObject GetCapabilityInfo(int index) const;
  Q_INVOKABLE void UseCapability(int index, const QVariantMap& state);
  Q_INVOKABLE void Test(const CapabilityObject& c) const;
  Q_INVOKABLE void RequestData(const QString& device_id);
  Q_INVOKABLE QVariant GetValue(int index) const;

signals:
  void dataLoaded();
  void dataLoadingFailed();
  void dataUpdated();

private:
  const QString kUnsupportedDelegate = "qrc:/controls/Unsupported.qml";
  const QMap<CapabilityType, QString> kDelegates = {
    { CapabilityType::OnOff,        "qrc:/controls/OnOff.qml" },
    { CapabilityType::VideoStream,  kUnsupportedDelegate },
    { CapabilityType::ColorSetting, kUnsupportedDelegate },
    { CapabilityType::Mode,         "qrc:/controls/Mode.qml" },
    { CapabilityType::Range,        "qrc:/controls/Range.qml" },
    { CapabilityType::Toggle,       "qrc:/controls/Toggle.qml" },
  };

  QString device_id_;
  YandexHomeApi *api_;
  QList<CapabilityObject> capabilities_;
  QTimer timer_;

private slots:
  void OnDeviceInfoReceived(const DeviceObject& info);
  void OnDeviceInfoReceivingFailed(const QString& message);
};
