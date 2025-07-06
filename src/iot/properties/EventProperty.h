#pragma once

#include "../common/IotObject.h"

class EventProperty : public IotObject {
  Q_OBJECT
public:
  explicit EventProperty(QObject* parent = nullptr);

  void SetValue(const QVariant &value) override;

  QVariant GetValue() const override;
};
