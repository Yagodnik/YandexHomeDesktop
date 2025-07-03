#include "RangeCapability.h"

RangeCapability::RangeCapability(QObject *parent) : ICapability("range", parent) {}

void RangeCapability::SetValue(const QVariant& value) {
  if (GetValue().toDouble() == value.toDouble()) {
    return;
  }

  state_["value"] = value.toDouble();

  emit valueChanged();
  emit stateChanged();
}

QVariant RangeCapability::GetValue() const {
  if (state_.isEmpty()) {
    return 0;
  }

  return state_["value"].toDouble();
}

double RangeCapability::GetMin() const {
  const auto range = parameters_["range"].toMap();
  return range["min"].toDouble();
}

double RangeCapability::GetMax() const {
  const auto range = parameters_["range"].toMap();
  return range["max"].toDouble();
}

double RangeCapability::GetPrecision() const {
  const auto range = parameters_["range"].toMap();
  return range["precision"].toDouble();
}

QVariantMap RangeCapability::Create(double value) {
  const auto instance = state_["instance"].toString();

  return {
    { "instance", instance },
    { "value", value }
  };
}

void RangeCapability::SetMin(double value) {
  QVariantMap range = parameters_.value("range").toMap();
  range["min"] = value;
  parameters_["range"] = range;

  emit parametersChanged();
  emit minChanged();
}

void RangeCapability::SetMax(double value) {
  QVariantMap range = parameters_.value("range").toMap();
  range["max"] = value;
  parameters_["range"] = range;

  emit parametersChanged();
  emit maxChanged();
}

void RangeCapability::SetPrecision(double value) {
  QVariantMap range = parameters_.value("range").toMap();
  range["precision"] = value;
  parameters_["range"] = range;

  emit parametersChanged();
  emit precisionChanged();
}

void RangeCapability::SetParameters(const QVariantMap &parameters) {
  ICapability::SetParameters(parameters);

  emit minChanged();
  emit maxChanged();
  emit precisionChanged();
}