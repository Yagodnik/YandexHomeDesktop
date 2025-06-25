#include "ICapability.h"

ICapability::ICapability(QObject *parent) : QObject(parent) {}

QVariantMap ICapability::GetState() const {
  return state_;
}

QVariantMap ICapability::GetParameters() const {
  return parameters_;
}

void ICapability::SetState(const QVariantMap &state) {
  state_ = state;

}
