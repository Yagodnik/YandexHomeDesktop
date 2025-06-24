#include "RoomsModel.h"

RoomsModel::RoomsModel(YandexHomeApi *api, QObject *parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
    &YandexHomeApi::userInfoReceived,
    this,
    &RoomsModel::OnUserInfoReceived);

  connect(api_,
    &YandexHomeApi::userInfoReceivingFailed,
    this,
    &RoomsModel::OnUserInfoReceivingFailed);
}

int RoomsModel::rowCount(const QModelIndex &parent) const {
  return rooms_.size();
}

QVariant RoomsModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= rooms_.size()) {
    return {};
  }

  const auto& room = rooms_.at(index.row());

  switch (role) {
    case NameRole:
      return room.name;
    case IdRole:
      return room.id;
    case HouseholdIdRole:
      return room.household_id;
    default:
      return {};
  }
}

QHash<int, QByteArray> RoomsModel::roleNames() const {
  return {
    { NameRole, "name" },
    { IdRole, "roomId" },
    { HouseholdIdRole, "householdId" }
  };
}

void RoomsModel::RequestData() {
  beginResetModel();

  rooms_.clear();

  endResetModel();

  api_->GetUserInfo();
}

void RoomsModel::OnUserInfoReceived(const UserInfo &info) {
  beginResetModel();

  rooms_.clear();

  rooms_ = info.rooms;

  endResetModel();

  emit dataLoaded();
}

void RoomsModel::OnUserInfoReceivingFailed(const QString &message) {
  qDebug() << "Rooms Model: Error receiving rooms:" << message;

  emit dataLoadingFailed();
}


