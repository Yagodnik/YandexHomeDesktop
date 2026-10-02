#pragma once

#include <QAbstractListModel>

#include "models/HomeSnapshotLoader.h"

class RoomsModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit RoomsModel(HomeSnapshotLoader *loader, QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    HouseholdIdRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void RequestData();

signals:
  void dataLoaded();
  void dataLoadingFailed();

private:
  HomeSnapshotLoader *loader_;
  QList<RoomObject> rooms_;

private slots:
  void OnUserInfoReceived(const UserInfo& info);
  void OnUserInfoReceivingFailed(const QString& message);
};
