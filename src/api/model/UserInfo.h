#pragma once

#include "serialization/Serialization.h"

JSON_ENUMERATION(Status,
  ("ok", Ok),
  ("error", Error)
);

JSON_ENUMERATION(DeviceType,
  ("devices.types.{type}", Lamp),
  ("devices.types.{type2}", Lamp2)
);

JSON_ENUMERATION(CapabilityType,
  ("devices.capabilities.{type}", Cap1),
  ("devices.capabilities.{type2}", Cap2)
);

JSON_STRUCT(RoomObject,
  (QString, id),
  (QString, name),
  (QString, household_id),
  (QStringList, devices)
);

JSON_STRUCT(CapabilityObject,
  (bool, retrievable),
  (QString, type)
  /* TODO: Parameters + State */
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
  (QString, id)
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
  (QString, request_id),
  (QList<RoomObject>, rooms),
  (QList<GroupObject>, groups),
  (QList<DeviceObject>, devices),
  (QList<ScenarioObject>, scenarios),
  (QList<HouseholdObject>, households)
);