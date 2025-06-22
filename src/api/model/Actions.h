#pragma once

#include "Capabilites.h"
#include "serialization/Serialization.h"

JSON_STRUCT(DeviceActionsObject,
  (QString, id),
  (QList<CapabilityObject>, actions)
);

JSON_STRUCT(DeviceActionObject,
  (QList<DeviceActionsObject>, devices)
);