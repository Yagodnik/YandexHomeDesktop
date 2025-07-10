#pragma once

#include "../common/IotObject.h"

class ModesCapability : public IotObject {
  Q_OBJECT
public:
  explicit ModesCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(const QString& value);
};

