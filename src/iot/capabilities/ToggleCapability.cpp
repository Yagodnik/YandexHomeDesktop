#include "ToggleCapability.h"
#include "iot/core/CapabilityState.h"

#include <QVariant>

ToggleCapability::ToggleCapability(QObject *parent) : IotObject("toggle", parent) {}

void ToggleCapability::SetValue(const QVariant &value) {
  if (GetValue() == value) {
    return;
  }

  SetStateValue(value);
}

QVariant ToggleCapability::GetValue() const {
  if (state_.isEmpty()) {
    return false;
  }

  return state_["value"].toBool();
}

QVariantMap ToggleCapability::Create(const bool value) {
  return Iot::State::Toggle(GetInstance(), value);
}
