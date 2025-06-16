#pragma once
#include "serialization/Serialization.h"

JSON_STRUCT(Response,
  (QString, request_id),
  (QString, status),
  (QString, message)
);