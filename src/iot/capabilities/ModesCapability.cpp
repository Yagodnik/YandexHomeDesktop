#include "ModesCapability.h"

ModesCapability::ModesCapability(QObject *parent)
  : IotObject("modes", parent) {}

void ModesCapability::SetValue(const QVariant &value) {
  if (GetValue().toString() == value.toString()) {
    return;
  }

  state_["value"] = value.toString();

  emit valueChanged();
  emit stateChanged();
}

QVariant ModesCapability::GetValue() const {
  if (state_.isEmpty()) {
    return 0;
  }

  return state_["value"];
}

QVariantMap ModesCapability::Create(double value) {
  const auto instance = state_["instance"].toString();

  return {
    { "instance", instance },
    { "value", value }
  };
}
