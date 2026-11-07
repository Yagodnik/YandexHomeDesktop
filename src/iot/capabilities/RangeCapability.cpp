#include "RangeCapability.h"
#include "iot/core/CapabilityState.h"
#include "iot/core/CapabilityParameters.h"

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

bool RangeCapability::GetRandomAccessSupport() const {
  return Iot::RangeParameters(parameters_).RandomAccess();
}

double RangeCapability::GetMin() const {
  return Iot::RangeParameters(parameters_).Limits().Min();
}

double RangeCapability::GetMax() const {
  return Iot::RangeParameters(parameters_).Limits().Max();
}

double RangeCapability::GetPrecision() const {
  return Iot::RangeParameters(parameters_).Limits().Precision();
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
  return Iot::State::Range(GetInstance(), value);
}

QVariantMap RangeCapability::CreateRelative(double delta) {
  return Iot::State::RelativeRange(GetInstance(), delta);
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
  emit randomAccessSupportChanged();
}

void RangeCapability::SetUnitList(UnitsList *units_list) {
  if (units_list_ == units_list) {
    return;
  }

  units_list_ = units_list;
  emit unitsListChanged();
  emit unitChanged();
}
