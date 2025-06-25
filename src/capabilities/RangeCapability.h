#pragma once

#include "ICapability.h"

class RangeCapability : public ICapability {
  Q_OBJECT
  Q_PROPERTY(double min READ GetMin WRITE SetMin NOTIFY minChanged)
  Q_PROPERTY(double max READ GetMax WRITE SetMax NOTIFY maxChanged)
  Q_PROPERTY(double precision READ GetPrecision WRITE SetPrecision NOTIFY precisionChanged)
public:
  explicit RangeCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  [[nodiscard]] double GetMin() const;
  [[nodiscard]] double GetMax() const;
  [[nodiscard]] double GetPrecision() const;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(double value);

signals:
  void minChanged();
  void maxChanged();
  void precisionChanged();

public slots:
  void SetMin(double value);
  void SetMax(double value);
  void SetPrecision(double value);
  void SetParameters(const QVariantMap& parameters) override;
};

