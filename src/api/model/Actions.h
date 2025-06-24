#pragma once

#include "Capabilites.h"
#include "Status.h"
#include "serialization/Serialization.h"

JSON_STRUCT(DeviceActionsObject,
  (QString, id),
  (QList<CapabilityObject>, actions)
);

JSON_STRUCT(DeviceActionObject,
  (QList<DeviceActionsObject>, devices)
);

JSON_STRUCT(ActionResult,
  (QString, status),
  (QString, error_code),
  (QString, error_message)
);

JSON_STRUCT(CapabilityResponseState,
  (QString, instance),
  (ActionResult, action_result)
);

JSON_STRUCT(CapabilityResponse,
  (QString, type),
  (CapabilityResponseState, state)
);

JSON_STRUCT(DeviceActionsResponse,
  (QString, id),
  (QList<CapabilityResponse>, capabilities)
);

JSON_STRUCT(DeviceActionResponse,
  (Status, status),
  (QString, message),
  (QString, request_id),
  (QList<DeviceActionsResponse>, devices)
);
