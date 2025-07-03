#include "ICapability.h"

ICapability::ICapability(const QString& capability_name, QObject *parent)
  : QObject(parent), capability_name_(capability_name) {}

QVariantMap ICapability::GetState() const {
  return state_;
}

QVariantMap ICapability::GetParameters() const {
  return parameters_;
}

QString ICapability::GetCapabilityTitle() const {
  qDebug() << "Called by " << capability_name_;

  if (titles_list_ != nullptr) {
    const auto instance = state_.value("instance", "").toString();
    return titles_list_->GetTitle(capability_name_, instance);
  }

  return state_.value("instance", "").toString() + "???";
}

void ICapability::SetState(const QVariantMap &state) {
  const QVariant old_value = GetValue();
  state_ = state;
  emit stateChanged();

  // TODO: Maybe remove due useless
  emit capabilityTitleChanged();

  if (GetValue() != old_value) {
    emit valueChanged();
  }
}

void ICapability::SetParameters(const QVariantMap &parameters) {
  parameters_ = parameters;

  emit parametersChanged();
}
