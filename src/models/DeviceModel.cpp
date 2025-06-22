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
      return capability.type;
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

void DeviceModel::UseCapability(const int index, const QVariant &action_data) {
  qDebug() << "Going to implement soon!";
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

void DeviceModel::OnDeviceInfoReceived(const DeviceObject &info) {
  beginResetModel();

  capabilities_.clear();
  capabilities_ = info.capabilities;

  qDebug() << "DeviceModel::OnDeviceInfoReceived" << capabilities_.size();

  endResetModel();

  emit dataLoaded();
}

void DeviceModel::OnDeviceInfoReceivingFailed(const QString &message) {
  qDebug() << "Devices Model: Error receiving devices:" << message;

  emit dataLoadingFailed();
}
