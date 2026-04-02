#include "DevicesFilterModel.h"

#include "DevicesModel.h"

DevicesFilterModel::DevicesFilterModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
  connect(this, &QAbstractItemModel::rowsInserted, this, &DevicesFilterModel::countChanged);
  connect(this, &QAbstractItemModel::rowsRemoved,  this, &DevicesFilterModel::countChanged);
  connect(this, &QAbstractItemModel::modelReset,   this, &DevicesFilterModel::countChanged);
}

QString DevicesFilterModel::householdId() const {
  return household_id_;
}

QString DevicesFilterModel::roomId() const {
  return room_id_;
}

int DevicesFilterModel::GetCount() const {
  return rowCount();
}

QString DevicesFilterModel::deviceName() const {
  return device_name_;
}

void DevicesFilterModel::setHouseholdId(const QString &id) {
  if (id != household_id_) {
    beginFilterChange();

    household_id_ = id;
    emit householdIdChanged();

    endFilterChange();
  }
}

void DevicesFilterModel::setRoomId(const QString &id) {
  if (id != room_id_) {
    beginFilterChange();

    room_id_ = id;
    emit roomIdChanged();

    endFilterChange();
  }
}

void DevicesFilterModel::setDeviceName(const QString &name) {
  if (name == device_name_) {
    beginFilterChange();

    device_name_ = name;
    emit deviceNameChanged();

    endFilterChange();
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
  const auto device_name = sourceModel()->data(index,
    DevicesModel::NameRole).toString();

  const QString device_name_norm = device_name.normalized(QString::NormalizationForm_D);
  const QString search_name_norm = device_name_.normalized(QString::NormalizationForm_D);

  bool filtered = (household_id == household_id_) && (room_id == room_id_);

  if (!device_name_.isEmpty()) {
    // TODO: Improve filtering algorithm
    filtered &= device_name_norm.contains(search_name_norm, Qt::CaseInsensitive);
  }

  return filtered;
}
