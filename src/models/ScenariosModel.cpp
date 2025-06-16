#include "ScenariosModel.h"

ScenariosModel::ScenariosModel(YandexHomeApi *api, QObject *parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
          &YandexHomeApi::scenariosReceived,
          this,
          &ScenariosModel::OnScenariosReceived);
}

int ScenariosModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return scenarios_.size();
}

QVariant ScenariosModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= 10)
    return {};

  const auto& scenario = scenarios_[index.row()];
  switch (role) {
    case NameRole:
      return scenario.name;
    case ActingRole:
      qDebug() << scenario.name << " / " << scenario.is_active;
      return scenario.is_active;
    case IdRole:
      return scenario.id;
    default:
      return {};
  }
}

QHash<int, QByteArray> ScenariosModel::roleNames() const {
  return {
  { NameRole, "name" },
  { ActingRole, "acting" },
  { IdRole, "scenario_id" }
  };
}

void ScenariosModel::RequestData() const {
  api_->GetScenarios();
}

void ScenariosModel::OnScenariosReceived(const QList<ScenarioObject> &scenarios) {
  beginResetModel();

  scenarios_ = scenarios;

  endResetModel();
}
