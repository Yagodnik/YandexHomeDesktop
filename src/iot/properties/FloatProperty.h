#pragma once

#include "../common/IotObject.h"

class FloatProperty : public IotObject {
  Q_OBJECT
  Q_PROPERTY(QString unit READ GetUnit NOTIFY unitChanged)
  Q_PROPERTY(UnitsList* units READ GetUnitList WRITE SetUnitList NOTIFY unitsListChanged)
public:
  explicit FloatProperty(QObject* parent = nullptr);

  void SetValue(const QVariant &value) override;

  QVariant GetValue() const override;

  [[nodiscard]] QString GetUnit() const;
  [[nodiscard]] UnitsList* GetUnitList() const;

signals:
  void unitsListChanged();
  void unitChanged();

public slots:
  void SetParameters(const QVariantMap& parameters) override;
  void SetUnitList(UnitsList* units_list);

  QString GetFormattedValue() const override;

protected:
  UnitsList* units_list_{nullptr};
};
