#include "DevicesFilterModel.h"

#include "DevicesModel.h"

DevicesFilterModel::DevicesFilterModel(QObject *parent)
  : QSortFilterProxyModel(parent) {}

QString DevicesFilterModel::householdId() const {
  return household_id_;
}

QString DevicesFilterModel::roomId() const {
  return room_id_;
}

void DevicesFilterModel::setHouseholdId(const QString &id) {
  if (id != household_id_) {
    household_id_ = id;
    emit householdIdChanged();
    invalidateFilter();
  }
}

void DevicesFilterModel::setRoomId(const QString &id) {
  if (id != room_id_) {
    room_id_ = id;
    emit roomIdChanged();
    invalidateFilter();
  }
}

bool DevicesFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const {
  const auto index = sourceModel()->index(row, 0, parent);

  if (!index.isValid()) {
    return false;
  }

  const auto household_id = sourceModel()->data(index,
    DevicesModel::HouseholdIdRole).toString();
  const auto room_id = sourceModel()->data(index,
    DevicesModel::RoomIdRole).toString();

  return (household_id == household_id_) && (room_id == room_id_);
}
