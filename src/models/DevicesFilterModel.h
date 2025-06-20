#pragma once

#include <QSortFilterProxyModel>

class DevicesFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString householdId READ householdId WRITE setHouseholdId NOTIFY householdIdChanged)
  Q_PROPERTY(QString roomId READ roomId WRITE setRoomId NOTIFY roomIdChanged)

public:
  explicit DevicesFilterModel(QObject *parent = nullptr);

  [[nodiscard]] QString householdId() const;
  [[nodiscard]] QString roomId() const;

public slots:
  void setHouseholdId(const QString &id);
  void setRoomId(const QString &id);

signals:
  void householdIdChanged();
  void roomIdChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

  QString household_id_;
  QString room_id_;
};

