#pragma once

#include "serialization/SerializationTypes.h"

JSON_STRUCT(KeybindTarget, (QString, device_id), (QString, capability_type), (QString, instance),
            (QString, action), (QString, value), (QString, scenario_id));

JSON_STRUCT(KeybindAssignment, (KeybindTarget, target), (int, combination), (QString, device_name),
            (QString, action_label));
