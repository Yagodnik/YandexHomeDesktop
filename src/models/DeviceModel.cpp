#include "DeviceModel.h"

DeviceModel::DeviceModel(YandexHomeApi *api, QObject *parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
    &YandexHomeApi::deviceInfoReceived,
    this,
    &DeviceModel::OnDeviceInfoReceived);

  connect(api_,
    &YandexHomeApi::deviceInfoReceivingFailed,
    this,
    &DeviceModel::OnDeviceInfoReceivingFailed);

  timer_.setInterval(3000);
  timer_.setSingleShot(false);

  connect(&timer_, &QTimer::timeout, [this]() {
    qDebug() << "Requesting update:" << QString::number(
      static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000,
      'f', 3
    );

    RequestData(device_id_);
  });
}

int DeviceModel::rowCount(const QModelIndex &parent) const {
  return capabilities_.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= capabilities_.size()) {
    return {};
  }

  const auto& capability = capabilities_.at(index.row());

  switch (role) {
    case IdRole:
      return device_id_;
    case NameRole:
      return CapabilityType::operator[](capability.type);
    case LastUpdateTimeRole:
      return capability.last_updated;
    case DelegateSourceRole: {
      if (kDelegates.contains(capability.type)) {
        return kDelegates[capability.type];
      }

      return kUnsupportedDelegate;
    }
    default:
      return {};
  }
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
  return {
    { IdRole, "deviceId" },
    { NameRole, "name"},
    { DelegateSourceRole, "delegateSource" }
  };
}

void DeviceModel::SetId(const QString &device_id) {
  device_id_ = device_id;
}

CapabilityObject DeviceModel::GetCapabilityInfo(const int index) const {
  return capabilities_.at(index);
}

void DeviceModel::UseCapability(const int index, const QVariantMap &state) {
  qDebug() << "Capability index:" << index;
  qDebug() << "Capability instance:" << state["instance"].toString();

  auto& capability = capabilities_[index];

  qDebug() << "Previously updated:" << QString::number(capability.last_updated, 'f', 3);

  const CapabilityObject action = {
    .type = capability.type,
    .state = state
  };

  const DeviceActionsObject action_object = {
    .id = device_id_,
    .actions = { action }
  };

  api_->PerformActions({
    action_object
  });

  capability.last_updated = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;

  qDebug() << "New update time:" << QString::number(capability.last_updated, 'f', 3);
}

void DeviceModel::Test(const CapabilityObject &c) const {
  qDebug() << "Testing capability type: " << c.type;
}

void DeviceModel::RequestData(const QString& device_id) {
  device_id_ = device_id;

  beginResetModel();

  capabilities_.clear();

  endResetModel();

  api_->GetDeviceInfo(device_id_);
}

QVariant DeviceModel::GetValue(int index) const {
  const auto& capability = capabilities_.at(index);

  return capability.state["value"];
}

void DeviceModel::OnDeviceInfoReceived(const DeviceObject &info) {
  beginResetModel();

  capabilities_.clear();
  capabilities_ = info.capabilities;

  endResetModel();

  qDebug() << "DeviceModel::OnDeviceInfoReceived" << capabilities_.size();

  emit dataLoaded();
  emit dataUpdated();
}

void DeviceModel::OnDeviceInfoReceivingFailed(const QString &message) {
  qDebug() << "Devices Model: Error receiving devices:" << message;

  emit dataLoadingFailed();
}
