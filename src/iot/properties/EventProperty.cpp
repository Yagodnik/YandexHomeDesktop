#include "EventProperty.h"

EventProperty::EventProperty(QObject *parent) : IotObject("event", parent) {}

void EventProperty::SetValue(const QVariant &value) {
}

QVariant EventProperty::GetValue() const {
}
