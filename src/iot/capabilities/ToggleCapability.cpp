#include "ToggleCapability.h"

#include <QVariant>

ToggleCapability::ToggleCapability(QObject *parent) : IotObject("toggle", parent) {}

void ToggleCapability::SetValue(const QVariant &value) {
  if (GetValue() == value) {
    return;
  }

  state_["value"] = value;

  emit valueChanged();
  emit stateChanged();
}

QVariant ToggleCapability::GetValue() const {
  if (state_.isEmpty()) {
    return false;
  }

  return state_["value"].toBool();
}

QVariantMap ToggleCapability::Create(const bool value) {
  const auto instance = state_["instance"].toString();

  return {
    { "instance", instance },
    { "value", value }
  };
}
