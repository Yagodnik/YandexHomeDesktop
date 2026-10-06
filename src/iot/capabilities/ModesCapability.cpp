#include "ModesCapability.h"
#include "iot/core/CapabilityState.h"

ModesCapability::ModesCapability(QObject *parent)
  : IotObject("mode", parent) {}

void ModesCapability::SetValue(const QVariant &value) {
  if (GetValue().toString() == value.toString()) {
    return;
  }

  SetStateValue(value);
}

QVariant ModesCapability::GetValue() const {
  if (state_.isEmpty()) {
    return 0;
  }

  return state_.value("value", "").toString();
}

QVariantMap ModesCapability::Create(const QString& value) {
  return Iot::State::Mode(GetInstance(), value);
}
