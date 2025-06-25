#include "ICapability.h"

ICapability::ICapability(QObject *parent) : QObject(parent) {}

QVariantMap ICapability::GetState() const {
  return state_;
}

QVariantMap ICapability::GetParameters() const {
  return parameters_;
}

void ICapability::SetState(const QVariantMap &state) {
  const QVariant old_value = GetValue();
  state_ = state;
  emit stateChanged();

  if (GetValue() != old_value) {
    emit valueChanged();
  }
}

void ICapability::SetParameters(const QVariantMap &parameters) {
  parameters_ = parameters;
  emit parametersChanged();
}
