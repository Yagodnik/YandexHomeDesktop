#pragma once

#include "yh/yandexapiadapter_export.h"

#include "serialization/SerializationTypes.h"

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

// Shared codecs are instantiated once in Serialization.cpp.
extern template YANDEXAPIADAPTER_EXPORT CapabilityObject Serialization::From<CapabilityObject>(const QJsonObject&);
extern template YANDEXAPIADAPTER_EXPORT QJsonObject Serialization::To<CapabilityObject>(const CapabilityObject&);
