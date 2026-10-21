#pragma once

#include "yh/iotcore_export.h"
#include "CapabilityRules.h"

namespace Iot {
IOTCORE_EXPORT bool IsFiniteNumber(const QVariant& value);
IOTCORE_EXPORT ValidationResult RequireInstance(const QVariantMap& state);
IOTCORE_EXPORT ValidationResult CheckLimits(const NumericLimits& limits, double value);
}
