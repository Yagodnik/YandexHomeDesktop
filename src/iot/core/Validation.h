#pragma once
#include "CapabilityRules.h"

namespace Iot {
bool IsFiniteNumber(const QVariant& value);
ValidationResult RequireInstance(const QVariantMap& state);
ValidationResult CheckLimits(const NumericLimits& limits, double value);
}
