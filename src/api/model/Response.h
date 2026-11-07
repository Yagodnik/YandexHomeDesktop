#pragma once

#include "yh/yandexapiadapter_export.h"

#include "serialization/SerializationTypes.h"
#include "Status.h"

JSON_STRUCT(Response,
  (QString, request_id),
  (Status, status),
  (QString, message)
);

// Shared codecs are instantiated once in Serialization.cpp.
extern template YANDEXAPIADAPTER_EXPORT Response Serialization::From<Response>(const QJsonObject&);
