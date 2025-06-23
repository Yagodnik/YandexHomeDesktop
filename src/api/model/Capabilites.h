#pragma once

#include "serialization/Serialization.h"

JSON_ENUMERATION(CapabilityType,
  ("devices.capabilities.on_off", OnOff),
  ("devices.capabilities.color_setting", ColorSetting),
  ("devices.capabilities.video_stream", VideoStream),
  ("devices.capabilities.mode", Mode),
  ("devices.capabilities.range", Range),
  ("devices.capabilities.toggle", Toggle)
);

JSON_STRUCT(CapabilityObject,
  (CapabilityType, type),
  (bool, retrievable),
  (QVariantMap, state),
  (QVariantMap, parameters),
  (double, last_updated)
);
