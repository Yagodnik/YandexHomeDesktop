#pragma once

#include <QAbstractListModel>
#include "api/model/UserInfo.h"

class ScenariosModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ Count NOTIFY countChanged)
public:
  struct ScenarioModel {
    ScenarioObject data;
    bool is_executing { false };
  };

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    IsWaitingResponseRole,
    IsActiveRole
  };

  explicit ScenariosModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  void SetScenarios(const QList<ScenarioObject>& scenarios);
  void SetExecuting(const QString& scenario_id, bool executing);

  [[nodiscard]] int Count() const;

signals:
  void countChanged();

private:
  QList<ScenarioModel> scenarios_;
};
