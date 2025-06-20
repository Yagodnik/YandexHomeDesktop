#include "DevicesModel.h"

DevicesModel::DevicesModel(YandexHomeApi *api, QObject *parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
    &YandexHomeApi::userInfoReceived,
    this,
    &DevicesModel::OnUserInfoReceived);

  connect(api_,
    &YandexHomeApi::userInfoReceivingFailed,
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
    default:
      return {};
  }
}

QHash<int, QByteArray> DevicesModel::roleNames() const {
  return {
  { NameRole, "name" },
  { IdRole, "device_id" },
  { RoomIdRole, "device_room_id" },
  { HouseholdIdRole, "device_household_id" }
  };
}

void DevicesModel::RequestData() {
  beginResetModel();

  devices_.clear();

  endResetModel();

  api_->GetUserInfo();
}

void DevicesModel::OnUserInfoReceived(const UserInfo &info) {
  beginResetModel();

  devices_.clear();

  devices_ = info.devices;

  endResetModel();

  emit dataLoaded();
}

void DevicesModel::OnUserInfoReceivingFailed(const QString &message) {
  qDebug() << "Devices Model: Error receiving devices:" << message;

  emit dataLoadingFailed();
}
