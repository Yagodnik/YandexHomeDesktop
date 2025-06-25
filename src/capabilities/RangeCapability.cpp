#include "RangeCapability.h"

RangeCapability::RangeCapability(QObject *parent) : ICapability(parent) {}

void RangeCapability::SetValue(const QVariant& value) {
  if (GetValue().toInt() == value.toInt()) {
    return;
  }

  state_["value"] = value.toInt();

  emit valueChanged();
  emit stateChanged();
}

QVariant RangeCapability::GetValue() const {
  if (state_.isEmpty()) {
    return 0;
  }

  return state_["value"].toInt();
}

QVariantMap RangeCapability::Create(int value) {
  const auto instance = state_["instance"].toString();

  return {
    { "instance", instance },
    { "value", value }
  };
}
