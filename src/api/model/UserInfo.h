#pragma once

#include "serialization/Serialization.h"
#include "Status.h"
#include "Capabilites.h"

JSON_ENUMERATION(DeviceType,
  ("devices.types.{type}", Lamp),
  ("devices.types.{type2}", Lamp2)
);


JSON_STRUCT(RoomObject,
  (QString, id),
  (QString, name),
  (QString, household_id),
  (QStringList, devices)
);


JSON_STRUCT(GroupObject,
  (QString, id),
  (QString, name),
  (QStringList, aliases),
  (QString, household_id),
  (QStringList, devices),
  (QList<CapabilityObject>, capabilities)
);

JSON_STRUCT(DeviceObject,
  (QString, id),
  (QString, name),
  (QStringList, aliases),
  (QString, room),
  (QString, household_id),
  (QList<CapabilityObject>, capabilities)
);

JSON_STRUCT(DeviceInfo,
  (Status, status),
  (QString, message),
  (QString, request_id),
  (QList<CapabilityObject>, capabilities)
);

JSON_STRUCT(ScenarioObject,
  (QString, id),
  (QString, name),
  (bool, is_active)
);

JSON_STRUCT(HouseholdObject,
  (QString, id),
  (QString, name)
);

JSON_STRUCT(UserInfo,
  (Status, status),
  (QString, message),
  (QString, request_id),
  (QList<RoomObject>, rooms),
  (QList<GroupObject>, groups),
  (QList<DeviceObject>, devices),
  (QList<ScenarioObject>, scenarios),
  (QList<HouseholdObject>, households)
);