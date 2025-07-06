#pragma once

#include <QMap>
#include "../common/IotObject.h"

class OnOffCapability : public IotObject {
  Q_OBJECT
public:
  explicit OnOffCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(bool data);
};
