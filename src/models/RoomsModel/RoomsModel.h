#pragma once

#include <QAbstractListModel>

#include "api/YandexHomeApi.h"
#include "api/model/UserInfo.h"
#include "infrastructure/settings/ISettingsStorage.h"

class RoomsModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit RoomsModel(
    YandexHomeApi *api,
    ISettingsStorage *settings,
    QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    HouseholdIdRole,
    Collapsed
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
  QList<RoomObject> rooms_;
  ISettingsStorage *settings_;

private slots:
  void OnUserInfoReceived(const UserInfo& info);
  void OnUserInfoReceivingFailed(const QString& message);
};

