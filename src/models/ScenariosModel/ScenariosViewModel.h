#pragma once

#include "models/ScenariosModel/ScenariosModel.h"
#include "services/ScenarioService.h"

class ScenariosViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(PageState state READ GetState NOTIFY stateChanged)
  Q_PROPERTY(bool loading READ IsLoading NOTIFY stateChanged)
  Q_PROPERTY(ScenariosModel* scenarios READ GetScenarios CONSTANT)
public:
  enum PageState { Loading, Error, Ready };
  Q_ENUM(PageState)

  explicit ScenariosViewModel(ScenarioService* service, QObject* parent = nullptr);

  [[nodiscard]] PageState GetState() const;
  [[nodiscard]] bool IsLoading() const;
  [[nodiscard]] ScenariosModel* GetScenarios();

  Q_INVOKABLE void EnsureLoaded();
  Q_INVOKABLE void Refresh();
  Q_INVOKABLE void ExecuteScenario(const QString& scenario_id);

signals:
  void stateChanged();
  void executionFailed(const QString& message);

private:
  void UpdateScenarios();

  ScenarioService* service_;
  ScenariosModel scenarios_;
};
