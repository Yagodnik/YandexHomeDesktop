#pragma once

#include <QHash>
#include <optional>

#include <QObject>
#include <QUuid>
#include "api/IHomeApi.h"

class ScenarioService : public QObject {
  Q_OBJECT
public:
  enum class LoadState { NotLoaded, Loading, Ready, Error };

  explicit ScenarioService(IHomeApi* api, QObject* parent = nullptr);

  [[nodiscard]] LoadState GetLoadState() const;
  [[nodiscard]] const QList<ScenarioObject>& GetScenarios() const;
  [[nodiscard]] bool IsExecuting(const QString& scenario_id) const;

  void EnsureLoaded();
  void Refresh();
  void ExecuteScenario(const QString& scenario_id);
  void Reset();
  void ListScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler);
  void RunScenario(const QString& id, QObject* context, ApiResultHandler<void> handler);

signals:
  void loadStateChanged();
  void scenariosChanged();
  void executionStateChanged(const QString& scenario_id);
  void executionFailed(const QString& scenario_id, const QString& message);

private:
  void SetLoadState(LoadState state);
  bool FinishExecution(const QString& scenario_id, const QUuid& request_id);

  IHomeApi* api_;
  LoadState load_state_ = LoadState::NotLoaded;
  QList<ScenarioObject> scenarios_;
  std::optional<QUuid> load_request_;
  QHash<QString, QUuid> executions_;
};
