#pragma once

#include "serialization/Serialization.h"
#include "Status.h"

JSON_STRUCT(Response,
  (QString, request_id),
  (Status, status),
  (QString, message)
);