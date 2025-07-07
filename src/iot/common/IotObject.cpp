#include "IotObject.h"

IotObject::IotObject(const QString& name, QObject *parent)
  : QObject(parent), name_(name) {}

QVariantMap IotObject::GetState() const {
  return state_;
}

QVariantMap IotObject::GetParameters() const {
  return parameters_;
}

QString IotObject::GetTitle() const {
  if (titles_provider_ != nullptr) {
    const auto instance = state_.value("instance", "").toString();
    return titles_provider_->GetTitle(name_, instance);
  }

  return state_.value("instance", "").toString() + "???";
}

QString IotObject::GetInstance() const {
  return state_.value("instance", "").toString();
}

void IotObject::SetState(const QVariantMap &state) {
  const QVariant old_value = GetValue();
  state_ = state;
  emit stateChanged();

  emit titleChanged();
  emit instanceChanged();

  if (GetValue() != old_value) {
    emit valueChanged();
    emit formattedValueChanged();
  }
}

void IotObject::SetParameters(const QVariantMap &parameters) {
  parameters_ = parameters;

  emit parametersChanged();
  emit formattedValueChanged();
}

QString IotObject::GetFormattedValue() const {
  return GetValue().toString();
}

void IotObject::SetStateValue(const QVariant &value) {
  state_["value"] = value;

  emit valueChanged();
  emit stateChanged();
}
