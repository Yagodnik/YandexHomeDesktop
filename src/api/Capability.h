#pragma once

#include <QMap>
#include <QString>

enum class CapabilityType {

};

struct Capability {
  bool retrievable;
  CapabilityType type;
  float last_updated;
};
