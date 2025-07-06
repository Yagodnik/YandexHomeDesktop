#include "EventProperty.h"

EventProperty::EventProperty(QObject *parent) : IotObject("event", parent) {}

void EventProperty::SetValue(const QVariant &value) {
  state_["value"] = value.toString();
}

QVariant EventProperty::GetValue() const {
  return state_.value("value", "").toString();
}
