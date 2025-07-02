#pragma once

#include "ICapability.h"

class ModesCapability : public ICapability {
  Q_OBJECT
public:
  explicit ModesCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(double value);
};

