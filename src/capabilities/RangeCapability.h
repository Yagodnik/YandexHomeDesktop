#pragma once

#include "ICapability.h"

class RangeCapability : public ICapability {
  Q_OBJECT
public:
  explicit RangeCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(int value);
};
