#include "HouseholdsModel.h"

HouseholdsModel::HouseholdsModel(QObject* parent) : QAbstractListModel(parent) {}

int HouseholdsModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : households_.size();
}

QVariant HouseholdsModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 ||
      index.row() >= households_.size() || index.column() != 0) {
    return {};
  }
  const auto& household = households_.at(index.row());
  switch (role) {
    case IdRole: return household.id;
    case NameRole: return household.name;
    default: return {};
  }
}

QHash<int, QByteArray> HouseholdsModel::roleNames() const {
  return {
    {IdRole, "householdId"},
    {NameRole, "name"}
  };
}

int HouseholdsModel::GetCount() const {
  return households_.size();
}

void HouseholdsModel::SetHouseholds(const QList<HouseholdObject>& items) {
  const auto old_count = GetCount();
  beginResetModel();
  households_ = items;
  endResetModel();
  if (old_count != GetCount()) {
    emit countChanged();
  }
}
