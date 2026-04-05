#include "RoomsModel.h"

RoomsModel::RoomsModel(YandexHomeApi *api, ISettingsStorage *settings, QObject *parent)
  : QAbstractListModel(parent), api_(api), settings_(settings)
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

  const QStringList collapsed = settings_->GetList("collapsed-rooms");
  const bool is_collapsed = collapsed.contains(room.id);

  switch (role) {
    case NameRole:
      return room.name;
    case IdRole:
      return room.id;
    case HouseholdIdRole:
      return room.household_id;
    case Collapsed:
      return is_collapsed;
    default:
      return {};
  }
}

QHash<int, QByteArray> RoomsModel::roleNames() const {
  return {
    { NameRole, "name" },
    { IdRole, "roomId" },
    { HouseholdIdRole, "householdId" },
    { Collapsed, "collapsed" }
  };
}

// TODO: Probably unused function, not sure
void RoomsModel::RequestData() {
  beginResetModel();

  qDebug() << "RoomsModel::RequestData";

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
  qWarning() << "RoomsModel: Error receiving rooms:" << message;

  emit dataLoadingFailed();
}


