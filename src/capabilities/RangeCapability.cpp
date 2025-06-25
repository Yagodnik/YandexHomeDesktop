#include "RangeCapability.h"

RangeCapability::RangeCapability(QObject *parent) : QObject(parent) {}

QVariantMap RangeCapability::GetState() const {
  return state_;
}

QVariantMap RangeCapability::GetParameters() const {
  return parameters_;
}

int RangeCapability::GetValue() const {
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

void RangeCapability::SetState(const QVariantMap &state) {
  const bool old_value = GetValue();
  state_ = state;
  emit stateChanged();

  if (GetValue() != old_value) {
    qDebug() << "Value update";
    emit valueChanged();
  }
}

void RangeCapability::SetParameters(const QVariantMap &parameters) {
  parameters_ = parameters;
  emit parametersChanged();
}

void RangeCapability::SetValue(const int value) {
  if (GetValue() == value) {
    return;
  }

  state_["value"] = value;

  emit valueChanged();
  emit stateChanged();
}
