#include "DevicesModel.h"

DevicesModel::DevicesModel(HomeSnapshotLoader *loader, QObject *parent)
  : QAbstractListModel(parent), loader_(loader)
{
  connect(loader_,
    &HomeSnapshotLoader::loaded,
    this,
    &DevicesModel::OnUserInfoReceived);

  connect(loader_,
    &HomeSnapshotLoader::failed,
    this,
    &DevicesModel::OnUserInfoReceivingFailed);
}

int DevicesModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return devices_.size();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= devices_.size()) {
    return {};
  }

  const auto& device = devices_.at(index.row());

  switch (role) {
    case NameRole:
      return device.name;
    case IdRole:
      return device.id;
    case RoomIdRole:
      return device.room;
    case HouseholdIdRole:
      return device.household_id;
    case TypeRole:
      return device.type;
    default:
      return {};
  }
}

QHash<int, QByteArray> DevicesModel::roleNames() const {
  return {
    { NameRole, "name" },
    { IdRole, "deviceId" },
    { RoomIdRole, "deviceRoomId" },
    { HouseholdIdRole, "deviceHouseholdId" },
    { TypeRole, "deviceType" }
  };
}

void DevicesModel::RequestData() {
  beginResetModel();

  devices_.clear();

  endResetModel();

  loader_->Refresh();
}

void DevicesModel::OnUserInfoReceived(const UserInfo &info) {
  beginResetModel();

  devices_.clear();

  devices_ = info.devices;

  endResetModel();

  emit dataLoaded();
}

void DevicesModel::OnUserInfoReceivingFailed(const QString &message) {
  qWarning() << "DevicesModel: Error receiving devices:" << message;

  emit dataLoadingFailed();
}
