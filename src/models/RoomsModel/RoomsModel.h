#pragma once

#include <QAbstractListModel>
#include "api/model/UserInfo.h"

class RoomsModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ GetCount NOTIFY countChanged)
public:
  explicit RoomsModel(QObject* parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    HouseholdIdRole
  };

  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] int GetCount() const;
  void SetRooms(const QList<RoomObject>& items);

signals:
  void countChanged();

private:
  QList<RoomObject> rooms_;
};
