#pragma once

#include <QSortFilterProxyModel>

#include "infrastructure/settings/ISettingsStorage.h"

class FavoriteDevicesModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(int count READ GetCount NOTIFY countChanged)
  Q_PROPERTY(ISettingsStorage* settingsStorage2 READ settingsStorage WRITE setSettingsStorage NOTIFY settingsStorageChanged)
public:
  explicit FavoriteDevicesModel(QObject *parent = nullptr);

  [[nodiscard]] int GetCount() const;
  [[nodiscard]] ISettingsStorage* settingsStorage() const;

  void setSettingsStorage(ISettingsStorage* settingsStorage);

signals:
  void countChanged();
  void settingsStorageChanged();

private:
  const QString kFavoriteDevicesList = "favorite-devices";

  ISettingsStorage* settings_storage_ = nullptr;

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
};
