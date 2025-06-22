#pragma once

#include "serialization/Serialization.h"

JSON_ENUMERATION(CapabilityType,
  ("devices.capabilities.{type}", Cap1),
  ("devices.capabilities.{type2}", Cap2)
);

JSON_STRUCT(CapabilityObject,
  (bool, retrievable),
  (QString, type),
  (float, last_updated)
  /* TODO: Parameters + State */
);
