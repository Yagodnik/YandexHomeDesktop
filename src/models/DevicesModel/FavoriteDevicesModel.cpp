#include "FavoriteDevicesModel.h"

#include "DevicesFilterModel.h"
#include "DevicesModel.h"
#include "infrastructure/settings/SettingsStorage.h"

FavoriteDevicesModel::FavoriteDevicesModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
  connect(this, &QAbstractItemModel::rowsInserted, this, &FavoriteDevicesModel::countChanged);
  connect(this, &QAbstractItemModel::rowsRemoved,  this, &FavoriteDevicesModel::countChanged);
  connect(this, &QAbstractItemModel::modelReset,   this, &FavoriteDevicesModel::countChanged);
}

int FavoriteDevicesModel::GetCount() const {
  return rowCount();
}

ISettingsStorage* FavoriteDevicesModel::settingsStorage() const {
  return settings_storage_;
}

void FavoriteDevicesModel::setSettingsStorage(ISettingsStorage *settingsStorage) {
  qDebug() << "FavoriteDevicesModel::setSettingsStorage";
  settings_storage_ = settingsStorage;
  emit settingsStorageChanged();
}

bool FavoriteDevicesModel::filterAcceptsRow(int row, const QModelIndex &parent) const {
  const auto index = sourceModel()->index(row, 0, parent);

  if (!index.isValid()) {
    return false;
  }

  if (settings_storage_ == nullptr) {
    qWarning() << "FavoriteDevicesModel: Nullptr settings storage!";
    return false;
  }

  qDebug() << "FavoriteDevicesModel: Filtering favorite devices";

  const QStringList favorite_devices = settings_storage_->GetList(kFavoriteDevicesList);
  const QString device_id = sourceModel()->data(index, DevicesModel::IdRole).toString();

  qDebug() << "FavoriteDevicesModel:" << device_id << "-" << favorite_devices.contains(device_id);

  return favorite_devices.contains(device_id);
}
