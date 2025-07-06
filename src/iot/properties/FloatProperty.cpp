#include "FloatProperty.h"

FloatProperty::FloatProperty(QObject *parent) : IotObject("float", parent) {}

void FloatProperty::SetValue(const QVariant &value) {
  state_["value"] = value.toDouble();

  emit stateChanged();
}

QVariant FloatProperty::GetValue() const {
  return state_.value("value", 0.f).toDouble();
}

QString FloatProperty::GetUnit() const {
  const auto instance = parameters_.value("unit", "").toString();
  if (units_list_ == nullptr) {
    return "?";
  }

  return units_list_->GetUnit(instance);
}

UnitsList* FloatProperty::GetUnitList() const {
  return units_list_;
}

void FloatProperty::SetParameters(const QVariantMap &parameters) {
  IotObject::SetParameters(parameters);

  emit unitChanged();
}

void FloatProperty::SetUnitList(UnitsList *units_list) {
  if (units_list_ == units_list) {
    return;
  }

  units_list_ = units_list;
  emit unitsListChanged();
  emit unitChanged();
}
