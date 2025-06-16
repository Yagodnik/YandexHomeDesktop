#pragma once

#include <QAbstractListModel>
#include "api/YandexHomeApi.h"

class ScenariosModel : public QAbstractListModel {
  Q_OBJECT
public:
  struct ScenarioModel {
    ScenarioObject data;
    mutable bool is_executing;
  };

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    IsWaitingResponseRole
  };

  explicit ScenariosModel(YandexHomeApi *api, QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void RequestData() const;
  Q_INVOKABLE void ExecuteScenario(int index);

signals:
  void dataLoaded();

private:
  YandexHomeApi *api_;
  QList<ScenarioModel> scenarios_;

private slots:
  void OnScenariosReceived(const QList<ScenarioObject> &scenarios);
  void OnScenarioFinished(const QString& scenario_id);
};
