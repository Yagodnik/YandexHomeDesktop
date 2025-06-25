#pragma once

#include <QMap>
#include "ICapability.h"

class ToggleCapability : public ICapability {
  Q_OBJECT
public:
  explicit ToggleCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(bool value);
};
