#include "HouseholdsModel.h"
#include <QDebug>

HouseholdsModel::HouseholdsModel(HomeSnapshotLoader *loader, QObject *parent)
  : QAbstractListModel(parent), current_(std::nullopt), loader_(loader)
{
  connect(loader_,
    &HomeSnapshotLoader::loaded,
    this,
    &HouseholdsModel::OnUserInfoReceived);

  connect(loader_,
    &HomeSnapshotLoader::failed,
    this,
    &HouseholdsModel::OnUserInfoReceivingFailed);
}

int HouseholdsModel::rowCount(const QModelIndex &parent) const {
  return households_.size();
}

QVariant HouseholdsModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= households_.size()) {
    return {};
  }

  const auto& scenario = households_[index.row()];
  switch (role) {
    case IdRole:
      return scenario.id;
    case NameRole:
      return scenario.name;
    default:
      return {};
  }
}

QHash<int, QByteArray> HouseholdsModel::roleNames() const {
  return {
    { IdRole, "householdId" },
    { NameRole, "name"}
  };
}

int HouseholdsModel::GetCount() const {
  return households_.size();
}

QString HouseholdsModel::GetCurrentHousehold() const {
  if (!current_.has_value()) {
    return {};
  }

  return current_.value();
}

QString HouseholdsModel::GetCurrentHouseholdName() const {
  if (!current_.has_value()) {
    qWarning() << "HouseholdsModel: Cant find household with empty id!";
    return {};
  }

  for (const auto& household : households_) {
    if (household.id == current_.value()) {
      return household.name;
    }
  }

  qWarning() << "HouseholdsModel: Cant find household with id:" << current_.value();

  return {};
}

void HouseholdsModel::RequestData() {
  beginResetModel();

  households_.clear();

  endResetModel();

  loader_->Refresh();
}

void HouseholdsModel::SetCurrentHousehold(const QString &value) {
  if (current_ == value) {
    return;
  }

  current_ = value;
  emit currentHouseholdChanged();
}

void HouseholdsModel::OnUserInfoReceived(const UserInfo &info) {
  beginResetModel();

  households_.clear();

  households_ = info.households;
  emit countChanged();

  if (!households_.empty() && !current_.has_value()) {
    current_ = households_.front().id;
    emit currentHouseholdChanged();
  }

  endResetModel();

  emit dataLoaded();
}

void HouseholdsModel::OnUserInfoReceivingFailed(const QString &message) {
  qWarning() << "Rooms Model: Error receiving rooms:" << message;

  emit dataLoadingFailed();
}
