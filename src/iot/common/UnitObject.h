#pragma once
#include "IotObject.h"

class UnitObject : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString unit READ GetUnit NOTIFY unitChanged)
  Q_PROPERTY(UnitsList* units READ GetUnitList WRITE SetUnitList NOTIFY unitsListChanged)
public:
  explicit UnitObject(QObject* parent = nullptr);

  [[nodiscard]] QString GetUnit() const;
  [[nodiscard]] UnitsList* GetUnitList() const;

signals:
  void unitsListChanged();
  void unitChanged();

public slots:
  void SetUnitList(UnitsList* units_list);

protected:
  UnitsList* units_list_{nullptr};
};
