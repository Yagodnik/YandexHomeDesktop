#include "ScenariosModel.h"

ScenariosModel::ScenariosModel(YandexHomeApi *api, QObject *parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
    &YandexHomeApi::scenariosReceivedSuccessfully,
    this,
    &ScenariosModel::OnScenariosReceived);

  connect(api_,
    &YandexHomeApi::scenariosReceivingFailed,
    this,
    &ScenariosModel::OnScenariosReceivingFailed);

  connect(api_,
    &YandexHomeApi::scenarioExecutionFinishedSuccessfully,
    this,
    &ScenariosModel::OnScenarioExecutionFinishedSuccessfully);

  connect(api_,
    &YandexHomeApi::scenarioExecutionFailed,
    this,
    &ScenariosModel::OnScenarioExecutionFailed);
}

int ScenariosModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return scenarios_.size();
}

QVariant ScenariosModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= scenarios_.size()) {
    return {};
  }

  const auto& scenario = scenarios_[index.row()];
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
    { NameRole, "name" },
    { IdRole, "scenario_id" },
    { IsWaitingResponseRole, "is_waiting_response" },
    { IsActiveRole, "is_active"}
  };
}

void ScenariosModel::RequestData() {
  beginResetModel();

  scenarios_.clear();

  endResetModel();

  api_->GetScenarios();
}

void ScenariosModel::ExecuteScenario(int index) {
  if (index < 0 || index >= scenarios_.size()) {
    return;
  }

  auto& scenario = scenarios_.at(index);
  scenario.is_executing = true;

  const QModelIndex model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index, {IsWaitingResponseRole});

  const QString scenario_id = scenario.data.id;
  const QVariant user_data = index;

  api_->ExecuteScenario(scenario_id, user_data);
}

int ScenariosModel::Count() const {
  return scenarios_.size();
}

void ScenariosModel::OnScenariosReceived(const QList<ScenarioObject> &scenarios) {
  beginResetModel();

  scenarios_.clear();

  for (const auto& scenario : scenarios) {
    scenarios_.append({
      .data = scenario,
      .is_executing = false
    });
  }

  endResetModel();

  emit dataLoaded();
}

void ScenariosModel::OnScenariosReceivingFailed(const QString &message) {
  emit dataLoadingFailed();

  qDebug() << "Scenarios Model: Loading scenarios failed:" << message;
}

void ScenariosModel::OnScenarioExecutionFinishedSuccessfully(
  const QString &scenario_id, const QVariant &user_data
) {
  const int index = user_data.toInt();
  if (index < 0 || index >= scenarios_.size()) {
    return;
  }

  auto& scenario = scenarios_[index];

  scenario.is_executing = false;

  const QModelIndex updated_index = createIndex(index, 0);
  emit dataChanged(updated_index, updated_index, {IsWaitingResponseRole});

  qDebug() << "Scenarios Model: Scenario" << scenario_id << "(" << index << ") finished executing.";
}

void ScenariosModel::OnScenarioExecutionFailed(
  const QString &message, const QVariant &user_data
) {
  const int index = user_data.toInt();
  if (index < 0 || index >= scenarios_.size()) {
    return;
  }

  auto& scenario = scenarios_.at(index);

  scenario.is_executing = false;

  const QModelIndex updated_index = createIndex(index, 0);
  emit dataChanged(updated_index, updated_index, {IsWaitingResponseRole});

  qDebug() << "Scenarios Model: Scenario" << message << "(" << index << ") failed.";
  emit scenarioExecutionFailed();
}
