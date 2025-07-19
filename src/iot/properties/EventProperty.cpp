#include "EventProperty.h"

EventProperty::EventProperty(QObject *parent) : IotObject("event", parent) {}

void EventProperty::SetValue(const QVariant &value) {
  SetStateValue(value);
  emit formattedValueChanged();
}

QVariant EventProperty::GetValue() const {
  return state_.value("value", "").toString();
}

QString EventProperty::GetFormattedValue() const {
  if (values_provider_ == nullptr) {
    qWarning() << "EventProperty: No values provided!";
    return "???";
  }

  const auto instance = GetInstance();
  const auto value = GetValue().toString();

  return values_provider_->GetTitle(instance, value);
}

TitlesProvider* EventProperty::GetValuesProvider() const {
  return values_provider_;
}

void EventProperty::SetValuesProvider(TitlesProvider *values_provider) {
  if (values_provider == values_provider_) {
    return;
  }

  values_provider_ = values_provider;
  emit valuesProviderChanged();
  emit formattedValueChanged();
}
