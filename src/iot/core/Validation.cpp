#include "Validation.h"
#include <cmath>

namespace Iot {
bool IsFiniteNumber(const QVariant& value) {
  const auto type = value.metaType().id();
  const bool numeric = type == QMetaType::Double || type == QMetaType::Float ||
    type == QMetaType::Int || type == QMetaType::UInt ||
    type == QMetaType::LongLong || type == QMetaType::ULongLong;
  return numeric && std::isfinite(value.toDouble());
}
ValidationResult RequireInstance(const QVariantMap& state) {
  if (state.value("instance").toString().isEmpty()) {
    return std::unexpected(CapabilityMessages::tr("Для этого умения требуется экземпляр."));
  }
  return {};
}
ValidationResult CheckLimits(const NumericLimits& limits, double value) {
  if (!limits.Contains(value)) {
    return std::unexpected(CapabilityMessages::tr("Значение выходит за допустимые границы устройства."));
  }
  return {};
}
ValidationResult ValidateInput(const ICapabilityRules& rules, const QVariantMap& state) {
  if (state.contains("relative") && !rules.SupportsRelative()) {
    return std::unexpected(CapabilityMessages::tr("Относительное изменение поддерживается только для range."));
  }
  return rules.ValidateValue(state);
}
}
