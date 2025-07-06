#include "OnOffCapability.h"

#include <QVariant>

OnOffCapability::OnOffCapability(QObject *parent) : IotObject("on_off", parent) {}

void OnOffCapability::SetValue(const QVariant &value) {
  if (GetValue() == value) {
    return;
  }

  state_["value"] = value;

  emit valueChanged();
  emit stateChanged();
}

QVariant OnOffCapability::GetValue() const {
  if (state_.isEmpty()) {
    return false;
  }

  return state_["value"].toBool();
}

QVariantMap OnOffCapability::Create(const bool value) {
  return {
    { "instance", "on" },
    { "value", value }
  };
}
