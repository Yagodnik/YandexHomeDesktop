#include "ScenariosViewModel.h"

ScenariosViewModel::ScenariosViewModel(ScenarioService* service, QObject* parent)
  : QObject(parent), service_(service), scenarios_(this)
{
  connect(service_, &ScenarioService::loadStateChanged, this, &ScenariosViewModel::stateChanged);
  connect(service_, &ScenarioService::scenariosChanged, this, &ScenariosViewModel::UpdateScenarios);
  connect(service_, &ScenarioService::executionStateChanged, this,
    [this](const QString& scenario_id) {
      scenarios_.SetExecuting(scenario_id, service_->IsExecuting(scenario_id));
    });
  connect(service_, &ScenarioService::executionFailed, this,
    [this](const QString&, const QString&) {
      emit executionFailed(tr("Не удалось выполнить сценарий"));
    });

  UpdateScenarios();
}

ScenariosViewModel::PageState ScenariosViewModel::GetState() const {
  switch (service_->GetLoadState()) {
    case ScenarioService::LoadState::NotLoaded:
    case ScenarioService::LoadState::Loading:
      return Loading;
    case ScenarioService::LoadState::Error:
      return Error;
    case ScenarioService::LoadState::Ready:
      return Ready;
  }
  return Error;
}

bool ScenariosViewModel::IsLoading() const {
  return GetState() == Loading;
}

ScenariosModel* ScenariosViewModel::GetScenarios() {
  return &scenarios_;
}

void ScenariosViewModel::EnsureLoaded() {
  service_->EnsureLoaded();
}

void ScenariosViewModel::Refresh() {
  service_->Refresh();
}

void ScenariosViewModel::ExecuteScenario(const QString& scenario_id) {
  service_->ExecuteScenario(scenario_id);
}

void ScenariosViewModel::UpdateScenarios() {
  scenarios_.SetScenarios(service_->GetScenarios());
  for (const auto& scenario : service_->GetScenarios()) {
    scenarios_.SetExecuting(scenario.id, service_->IsExecuting(scenario.id));
  }
}
