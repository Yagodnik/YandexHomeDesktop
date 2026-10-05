#include "RoomsModel.h"

RoomsModel::RoomsModel(QObject* parent) : QAbstractListModel(parent) {}

int RoomsModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : rooms_.size();
}

QVariant RoomsModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 ||
      index.row() >= rooms_.size() || index.column() != 0) {
    return {};
  }
  const auto& room = rooms_.at(index.row());
  switch (role) {
    case IdRole: return room.id;
    case NameRole: return room.name;
    case HouseholdIdRole: return room.household_id;
    default: return {};
  }
}

QHash<int, QByteArray> RoomsModel::roleNames() const {
  return {
    {IdRole, "roomId"},
    {NameRole, "name"},
    {HouseholdIdRole, "householdId"}
  };
}

int RoomsModel::GetCount() const {
  return rooms_.size();
}

void RoomsModel::SetRooms(const QList<RoomObject>& items) {
  const auto old_count = GetCount();
  beginResetModel();
  rooms_ = items;
  endResetModel();
  if (old_count != GetCount()) {
    emit countChanged();
  }
}
