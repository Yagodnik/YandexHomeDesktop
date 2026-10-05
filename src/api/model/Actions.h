#pragma once

#include "Capabilites.h"
#include "Status.h"
#include "serialization/SerializationTypes.h"

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

// Shared codecs are instantiated once in Serialization.cpp.
extern template ActionResult Serialization::From<ActionResult>(const QJsonObject&);
extern template CapabilityResponseState Serialization::From<CapabilityResponseState>(const QJsonObject&);
extern template CapabilityResponse Serialization::From<CapabilityResponse>(const QJsonObject&);
extern template DeviceActionsResponse Serialization::From<DeviceActionsResponse>(const QJsonObject&);
extern template DeviceActionResponse Serialization::From<DeviceActionResponse>(const QJsonObject&);
extern template QJsonObject Serialization::To<DeviceActionsObject>(const DeviceActionsObject&);
