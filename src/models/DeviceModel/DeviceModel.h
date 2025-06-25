#pragma once

#include <QAbstractListModel>
#include "api/YandexHomeApi.h"

struct CapabilityData {
  CapabilityObject data;

  CapabilityData() = default;
  explicit CapabilityData(const CapabilityObject& cap) : data(cap) {}

  bool is_pending = false;
  double action_start_time {};
  double action_finish_time {};

  void SetPending(bool value) {
    is_pending = value;
  }

  void SetStartTime() {
    action_start_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
  }

  void SetFinishTime() {
    action_finish_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
  }

  [[nodiscard]] bool IsInside(double time, double delta) const {
    return time >= (action_start_time - delta)
        && time < (action_finish_time + delta);
  }

  [[nodiscard]] bool IsInside(double time, double l_delta, double r_delta) const {
    return time >= (action_start_time - l_delta)
        && time < (action_finish_time + r_delta);
  }
};

class DeviceModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit DeviceModel(YandexHomeApi *api, QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    LastUpdateTimeRole,
    DelegateSourceRole,
    BusyRole,
    StateRole,
    ParametersRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void RequestData(const QString& device_id);

  Q_INVOKABLE [[nodiscard]] CapabilityObject GetCapabilityInfo(int index) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap GetState(int index) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap GetParameters(int index) const;
  Q_INVOKABLE void UseCapability(int index, const QVariantMap& state);

signals:
  void dataLoaded();
  void dataLoadingFailed();
  void dataUpdated(int index);
  void errorOccured(const QString& error_message);

private:
  const QString kUnsupportedDelegate = "qrc:/controls/Unsupported.qml";
  const QMap<CapabilityType, QString> kDelegates = {
    { CapabilityType::OnOff,        "qrc:/controls/OnOff.qml" },
    { CapabilityType::VideoStream,  kUnsupportedDelegate },
    { CapabilityType::ColorSetting, "qrc:/controls/ColorSetting.qml" },
    // { CapabilityType::ColorSetting, kUnsupportedDelegate },
    { CapabilityType::Mode,         "qrc:/controls/Mode.qml" },
    { CapabilityType::Range,        "qrc:/controls/Range.qml" },
    { CapabilityType::Toggle,       "qrc:/controls/Toggle.qml" },
  };

  QString device_id_;
  YandexHomeApi *api_;

  QList<CapabilityData> capabilities_data_;

  double last_update_start_time_;

  QTimer timer_;

private slots:
  void OnDeviceInfoReceived(const DeviceObject& info);
  void OnDeviceInfoReceivingFailed(const QString& message);

  void OnActionExecutionFinishedSuccessfully(const QVariant& user_data);
  void OnActionExecutionFailed(const QString& message, const QVariant& user_data);
};
