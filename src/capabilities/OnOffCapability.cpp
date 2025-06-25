#include "OnOffCapability.h"

#include <QVariant>

OnOffCapability::OnOffCapability(QObject *parent) : QObject(parent) {}

QVariantMap OnOffCapability::GetState() const {
  return state_;
}

QVariantMap OnOffCapability::GetParameters() const {
  return parameters_;
}

bool OnOffCapability::GetValue() const {
  if (state_.isEmpty()) {
    return false;
  }

  return state_["value"].toBool();
}

QVariantMap OnOffCapability::Create(bool value) {
  return {
    { "instance", "on" },
    { "value", value }
  };
}

void OnOffCapability::SetState(const QVariantMap &state) {
  const bool old_value = GetValue();
  state_ = state;
  emit stateChanged();

  if (GetValue() != old_value) {
    qDebug() << "Value update";
    emit valueChanged();
  }
}

void OnOffCapability::SetParameters(const QVariantMap &parameters) {
  parameters_ = parameters;
  emit parametersChanged();
}

void OnOffCapability::SetValue(const bool value) {
  if (GetValue() == value) {
    return;
  }

  state_["value"] = value;

  emit valueChanged();
  emit stateChanged();
}
