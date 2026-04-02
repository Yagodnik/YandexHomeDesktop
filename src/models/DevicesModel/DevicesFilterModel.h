#pragma once

#include <QSortFilterProxyModel>

class DevicesFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString householdId READ householdId WRITE setHouseholdId NOTIFY householdIdChanged)
  Q_PROPERTY(QString roomId READ roomId WRITE setRoomId NOTIFY roomIdChanged)
  Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)
  Q_PROPERTY(int count READ GetCount NOTIFY countChanged)

public:
  explicit DevicesFilterModel(QObject *parent = nullptr);

  [[nodiscard]] QString householdId() const;
  [[nodiscard]] QString roomId() const;
  [[nodiscard]] int GetCount() const;
  [[nodiscard]] QString deviceName() const;

public slots:
  void setHouseholdId(const QString &id);
  void setRoomId(const QString &id);
  void setDeviceName(const QString &name);

signals:
  void householdIdChanged();
  void roomIdChanged();
  void countChanged();
  void deviceNameChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

  QString household_id_;
  QString room_id_;
  QString device_name_;
};

