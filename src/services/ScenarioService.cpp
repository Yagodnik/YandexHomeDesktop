#include "ScenarioService.h"

#include <algorithm>
#include <QDebug>

ScenarioService::ScenarioService(IHomeApi* api, QObject* parent)
  : QObject(parent), api_(api) {}

ScenarioService::LoadState ScenarioService::GetLoadState() const {
  return load_state_;
}

const QList<ScenarioObject>& ScenarioService::GetScenarios() const {
  return scenarios_;
}

bool ScenarioService::IsExecuting(const QString& scenario_id) const {
  return executions_.contains(scenario_id);
}

void ScenarioService::EnsureLoaded() {
  if (load_state_ == LoadState::NotLoaded) {
    Refresh();
  }
}

void ScenarioService::Refresh() {
  if (load_state_ == LoadState::Loading) {
    return;
  }

  const auto request_id = QUuid::createUuid();
  load_request_ = request_id;
  SetLoadState(LoadState::Loading);
  ListScenarios(this, [this, request_id](ApiResult<QList<ScenarioObject>> result) {
    if (load_request_ != request_id) {
      return;
    }
    load_request_.reset();
    if (!result) {
      qWarning() << "ScenarioService: Loading failed:" << result.error().message;
      SetLoadState(LoadState::Error);
      return;
    }

    scenarios_ = std::move(*result);
    emit scenariosChanged();
    SetLoadState(LoadState::Ready);
  });
}

void ScenarioService::ExecuteScenario(const QString& scenario_id) {
  if (load_state_ != LoadState::Ready || IsExecuting(scenario_id)) {
    return;
  }

  const auto scenario = std::find_if(scenarios_.cbegin(), scenarios_.cend(),
    [&scenario_id](const ScenarioObject& item) { return item.id == scenario_id; });
  if (scenario == scenarios_.cend() || !scenario->is_active) {
    return;
  }

  const auto request_id = QUuid::createUuid();
  executions_.insert(scenario_id, request_id);
  emit executionStateChanged(scenario_id);
  RunScenario(scenario_id, this, [this, scenario_id, request_id](ApiResult<void> result) {
    if (!FinishExecution(scenario_id, request_id)) {
      return;
    }
    if (!result) {
      qWarning() << "ScenarioService: Execution failed:" << scenario_id << result.error().message;
      emit executionFailed(scenario_id, result.error().message);
    }
  });
}

void ScenarioService::ListScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) {
  api_->GetScenarios(context, std::move(handler));
}

void ScenarioService::RunScenario(const QString& id, QObject* context, ApiResultHandler<void> handler) {
  api_->ExecuteScenario(id, context, std::move(handler));
}

void ScenarioService::Reset() {
  // In-flight responses belong to the old session and will fail correlation.
  load_request_.reset();
  executions_.clear();
  scenarios_.clear();
  emit scenariosChanged();
  SetLoadState(LoadState::NotLoaded);
}

void ScenarioService::SetLoadState(LoadState state) {
  if (load_state_ != state) {
    load_state_ = state;
    emit loadStateChanged();
  }
}

bool ScenarioService::FinishExecution(const QString& scenario_id, const QUuid& request_id) {
  const auto execution = executions_.constFind(scenario_id);
  if (execution == executions_.cend() || execution.value() != request_id) {
    return false;
  }

  executions_.remove(scenario_id);
  emit executionStateChanged(scenario_id);
  return true;
}
