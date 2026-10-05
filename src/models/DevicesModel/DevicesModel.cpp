#include "DevicesModel.h"

DevicesModel::DevicesModel(QObject* parent) : QAbstractListModel(parent) {}

int DevicesModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : devices_.size();
}

QVariant DevicesModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 ||
      index.row() >= devices_.size() || index.column() != 0) {
    return {};
  }
  const auto& device = devices_.at(index.row());
  switch (role) {
    case IdRole: return device.id;
    case NameRole: return device.name;
    case RoomIdRole: return device.room;
    case HouseholdIdRole: return device.household_id;
    case TypeRole: return device.type;
    default: return {};
  }
}

QHash<int, QByteArray> DevicesModel::roleNames() const {
  return {
    {IdRole, "deviceId"},
    {NameRole, "name"},
    {RoomIdRole, "deviceRoomId"},
    {HouseholdIdRole, "deviceHouseholdId"},
    {TypeRole, "deviceType"}
  };
}

int DevicesModel::GetCount() const {
  return devices_.size();
}

void DevicesModel::SetDevices(const QList<DeviceObject>& items) {
  const auto old_count = GetCount();
  beginResetModel();
  devices_ = items;
  endResetModel();
  if (old_count != GetCount()) {
    emit countChanged();
  }
}
