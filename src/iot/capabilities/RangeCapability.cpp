#include "RangeCapability.h"

RangeCapability::RangeCapability(QObject *parent) : IotObject("range", parent) {}

void RangeCapability::SetValue(const QVariant& value) {
  if (GetValue().toDouble() == value.toDouble()) {
    return;
  }

  SetStateValue(value);
  emit formattedValueChanged();
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

QString RangeCapability::GetUnit() const {
  const auto instance = parameters_.value("unit", "").toString();
  if (units_list_ == nullptr) {
    return "?";
  }

  return units_list_->GetUnit(instance);
}

UnitsList * RangeCapability::GetUnitList() const {
  return units_list_;
}

QString RangeCapability::GetFormattedValue() const {
  return GetValue().toString() + GetUnit();
}

QVariantMap RangeCapability::Create(double value) {
  const auto instance = state_.value("instance", "").toString();

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
  IotObject::SetParameters(parameters);

  emit minChanged();
  emit maxChanged();
  emit precisionChanged();
  emit unitChanged();
}

void RangeCapability::SetUnitList(UnitsList *units_list) {
  if (units_list_ == units_list) {
    return;
  }

  units_list_ = units_list;
  emit unitsListChanged();
  emit unitChanged();
}
