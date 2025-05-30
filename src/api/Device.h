#pragma once

#include "Capability.h"
#include <QStringList>

enum class DeviceType {
  Sensor, SensorButton
};

enum class DeviceState {
  Online, Offline
};

struct Device {
  QString id;
  QString name;
  QStringList aliases;
  DeviceType type;
  DeviceState state;
  QStringList groups;
  QList<Capability> capabilities;
};
