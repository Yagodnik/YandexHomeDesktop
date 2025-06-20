#pragma once
#include <QSortFilterProxyModel>

class RoomsFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString householdId READ householdId WRITE setHouseholdId NOTIFY householdIdChanged)

public:
  explicit RoomsFilterModel(QObject *parent = nullptr);

  [[nodiscard]] QString householdId() const;

public slots:
  void setHouseholdId(const QString &id);

signals:
  void householdIdChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

  QString household_id_;
};
