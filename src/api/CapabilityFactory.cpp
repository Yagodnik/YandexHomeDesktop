#include "CapabilityFactory.h"

CapabilityFactory::CapabilityFactory(QObject *parent)
  : QObject(parent) {}

QVariantMap CapabilityFactory::CreateOnOff(const CapabilityObject &info, bool value) {
  if (info.state["instance"] == "on") {
    qDebug() << "CapabilityFactory::CreateOnOff: on";
  } else {
    qDebug() << "CapabilityFactory::CreateOnOff: ???";
  }

  return {
    { "instance", "on" },
    { "value", value }
  };
}

QVariantMap CapabilityFactory::CreateRange(const CapabilityObject &info, int value) {
  const auto instance = info.state["instance"].toString();

  return {
    { "instance", instance },
    { "value", value }
  };
}


