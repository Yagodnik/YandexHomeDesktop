#include "Validation.h"

namespace Iot::Rules {
namespace {
class RangeRules final : public ICapabilityRules {
public:
  CapabilityType Type() const override { return CapabilityType::Range; }
  bool SupportsRelative() const override { return true; }
  ValidationResult ValidateValue(const QVariantMap& state) const override {
    const auto instance = RequireInstance(state);
    if (!instance) { return instance; }
    if (!IsFiniteNumber(state.value("value"))) {
      return std::unexpected(CapabilityMessages::tr("Значение должно быть конечным числом."));
    }
    return {};
  }
  bool Supports(const CapabilityObject& capability, const QVariantMap& state) const override {
    return MatchesInstance(capability, state);
  }
  ValidationResult ValidateDevice(const QVariantMap& parameters, const QVariantMap& state) const override {
    if (state.value("relative").toBool()) { return {}; }
    const RangeParameters range(parameters);
    if (range.RelativeOnly()) {
      return std::unexpected(CapabilityMessages::tr("Это range поддерживает только относительное изменение."));
    }
    return CheckLimits(range.Limits(), state.value("value").toDouble());
  }
};
}
std::shared_ptr<const ICapabilityRules> Range() { return std::make_shared<RangeRules>(); }
}
