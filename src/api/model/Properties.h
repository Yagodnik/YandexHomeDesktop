#pragma once

#include "yh/yandexapiadapter_export.h"

#include "serialization/SerializationTypes.h"

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

// Shared codecs are instantiated once in Serialization.cpp.
extern template YANDEXAPIADAPTER_EXPORT PropertyObject Serialization::From<PropertyObject>(const QJsonObject&);
