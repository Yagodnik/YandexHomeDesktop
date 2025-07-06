#include "UnitObject.h"

UnitObject::UnitObject(QObject *parent) : QObject(parent) {}

QString UnitObject::GetUnit() const {
  const auto instance = parameters_.value("unit", "").toString();
  if (units_list_ == nullptr) {
    return "?";
  }

  return units_list_->GetUnit(instance);
}

UnitsList* UnitObject::GetUnitList() const {
  return units_list_;
}

void UnitObject::SetUnitList(UnitsList *units_list) {
  if (units_list_ == units_list) {
    return;
  }

  units_list_ = units_list;
  emit unitsListChanged();
  emit unitChanged();
}
