#pragma once

#include "serialization/Serialization.h"

JSON_ENUMERATION(PropertyType,
  ("devices.properties.float", Float),
  ("devices.properties.event", Event)
);

JSON_STRUCT(PropertyObject,
  (PropertyType, type),
  (bool, retrievable),
  (QVariantMap, state),
  (QVariantMap, parameters),
  (double, last_updated)
);
