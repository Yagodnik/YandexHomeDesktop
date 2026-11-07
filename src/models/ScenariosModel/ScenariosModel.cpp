#include "ScenariosModel.h"

ScenariosModel::ScenariosModel(QObject *parent) : QAbstractListModel(parent) {}

int ScenariosModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : scenarios_.size();
}

QVariant ScenariosModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.model() != this || index.row() < 0 ||
      index.row() >= scenarios_.size() || index.column() != 0) {
    return {};
  }

  const auto& scenario = scenarios_.at(index.row());
  switch (role) {
    case NameRole:
      return scenario.data.name;
    case IdRole:
      return scenario.data.id;
    case IsWaitingResponseRole:
      return scenario.is_executing;
    case IsActiveRole:
      return scenario.data.is_active;
    default:
      return {};
  }
}

QHash<int, QByteArray> ScenariosModel::roleNames() const {
  return {
    {NameRole, "name"},
    {IdRole, "scenario_id"},
    {IsWaitingResponseRole, "is_waiting_response"},
    {IsActiveRole, "is_active"}
  };
}

int ScenariosModel::Count() const {
  return scenarios_.size();
}

void ScenariosModel::SetScenarios(const QList<ScenarioObject>& scenarios) {
  const auto previous_count = Count();
  beginResetModel();
  scenarios_.clear();
  for (const auto& scenario : scenarios) {
    scenarios_.append({.data = scenario});
  }
  endResetModel();

  if (Count() != previous_count) {
    emit countChanged();
  }
}

void ScenariosModel::SetExecuting(const QString& scenario_id, bool executing) {
  for (int row = 0; row < scenarios_.size(); ++row) {
    auto& scenario = scenarios_[row];
    if (scenario.data.id == scenario_id && scenario.is_executing != executing) {
      scenario.is_executing = executing;
      const auto model_index = index(row);
      emit dataChanged(model_index, model_index, {IsWaitingResponseRole});
      return;
    }
  }
}
