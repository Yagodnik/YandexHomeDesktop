#pragma once

#include <QAbstractListModel>
#include "api/model/UserInfo.h"

class DevicesModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ GetCount NOTIFY countChanged)
public:
  explicit DevicesModel(QObject* parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    RoomIdRole,
    HouseholdIdRole,
    TypeRole,
    FavoriteRole
  };

  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] int GetCount() const;
  void SetDevices(const QList<DeviceObject>& items);
  void SetFavoriteDevices(const QStringList& ids);

signals:
  void countChanged();

private:
  QList<DeviceObject> devices_;
  QStringList favorite_ids_;
};
