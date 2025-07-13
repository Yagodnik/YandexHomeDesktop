#pragma once

#include "../common/IotObject.h"
#include "utils/UnitsList.h"

class RangeCapability : public IotObject {
  Q_OBJECT
  Q_PROPERTY(bool supportsRandomAccess READ GetRandomAccessSupport NOTIFY randomAccessSupportChanged)
  Q_PROPERTY(double min READ GetMin WRITE SetMin NOTIFY minChanged)
  Q_PROPERTY(double max READ GetMax WRITE SetMax NOTIFY maxChanged)
  Q_PROPERTY(double precision READ GetPrecision WRITE SetPrecision NOTIFY precisionChanged)
  Q_PROPERTY(QString unit READ GetUnit NOTIFY unitChanged)
  Q_PROPERTY(UnitsList* units READ GetUnitList WRITE SetUnitList NOTIFY unitsListChanged)
public:
  explicit RangeCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  [[nodiscard]] bool GetRandomAccessSupport() const;
  [[nodiscard]] double GetMin() const;
  [[nodiscard]] double GetMax() const;
  [[nodiscard]] double GetPrecision() const;
  [[nodiscard]] QString GetUnit() const;
  [[nodiscard]] UnitsList* GetUnitList() const;

  QString GetFormattedValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(double value);
  Q_INVOKABLE [[nodiscard]] QVariantMap CreateRelative(double delta);

signals:
  void minChanged();
  void maxChanged();
  void precisionChanged();
  void unitsListChanged();
  void unitChanged();
  void randomAccessSupportChanged();

public slots:
  void SetMin(double value);
  void SetMax(double value);
  void SetPrecision(double value);
  void SetParameters(const QVariantMap& parameters) override;
  void SetUnitList(UnitsList* units_list);

private:
  UnitsList* units_list_{nullptr};
};

