#include "DeviceFilterModel.h"

#include "DeviceModel.h"
#include "models/DevicesModel/DevicesModel.h"

DeviceFilterModel::DeviceFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {}

QString DeviceFilterModel::GetTargetType() const {
  return target_type_;
}

void DeviceFilterModel::SetTargetType(const QString &type) {
  if (target_type_ == type) {
    return;
  }

  target_type_ = type;
  emit targetTypeChanged();
}

bool DeviceFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const {
  const auto index = sourceModel()->index(row, 0, parent);

  if (!index.isValid()) {
    return false;
  }

  const auto type = sourceModel()->data(index, DeviceModel::AttributeTypeRole).toString();

  return type == target_type_;
}
