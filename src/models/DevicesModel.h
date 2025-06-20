#pragma once

#include <QAbstractListModel>
#include "api/YandexHomeApi.h"

class DevicesModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit DevicesModel(YandexHomeApi *api, QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    RoomIdRole,
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
  YandexHomeApi *api_;
  QList<DeviceObject> devices_;

private slots:
  void OnUserInfoReceived(const UserInfo& info);
  void OnUserInfoReceivingFailed(const QString& message);
};
