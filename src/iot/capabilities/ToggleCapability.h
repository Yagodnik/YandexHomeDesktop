#pragma once

#include <QMap>
#include "../common/IotObject.h"

class ToggleCapability : public IotObject {
  Q_OBJECT
public:
  explicit ToggleCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(bool value);
};
