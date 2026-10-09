#include "RoomsFilterModel.h"

#include "RoomsModel.h"

RoomsFilterModel::RoomsFilterModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
  setSortRole(RoomsModel::NameRole);
  sort(0);
}

QString RoomsFilterModel::householdId() const {
  return household_id_;
}

void RoomsFilterModel::setHouseholdId(const QString &id) {
  if (id != household_id_) {
    household_id_ = id;
    emit householdIdChanged();
    invalidateFilter();
  }
}

bool RoomsFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const {
  const auto index = sourceModel()->index(row, 0, parent);

  if (!index.isValid()) {
    return false;
  }

  const auto household_id = sourceModel()->data(index,
    RoomsModel::HouseholdIdRole).toString();

  return household_id == household_id_;
}

bool RoomsFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
  const auto comparison = QString::compare(sourceModel()->data(left, RoomsModel::NameRole).toString(),
    sourceModel()->data(right, RoomsModel::NameRole).toString(), Qt::CaseSensitive);
  if (comparison != 0) { return comparison < 0; }
  return sourceModel()->data(left, RoomsModel::IdRole).toString()
    < sourceModel()->data(right, RoomsModel::IdRole).toString();
}
