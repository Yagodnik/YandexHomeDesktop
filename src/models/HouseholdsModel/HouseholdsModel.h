#pragma once

#include <QAbstractListModel>
#include "api/model/UserInfo.h"

class HouseholdsModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ GetCount NOTIFY countChanged)
public:
  explicit HouseholdsModel(QObject* parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole
  };

  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] int GetCount() const;
  void SetHouseholds(const QList<HouseholdObject>& items);

signals:
  void countChanged();

private:
  QList<HouseholdObject> households_;
};
