#include "Validation.h"

namespace Iot::Rules {
namespace {
class ModeRules final : public ICapabilityRules {
public:
  CapabilityType Type() const override { return CapabilityType::Mode; }
  ValidationResult ValidateValue(const QVariantMap& state) const override {
    if (state.value("instance").toString().isEmpty() || state.value("value").toString().isEmpty()) {
      return std::unexpected(CapabilityMessages::tr("Для mode требуются непустые экземпляр и значение."));
    }
    return {};
  }
  bool Supports(const CapabilityObject& capability, const QVariantMap& state) const override {
    return MatchesInstance(capability, state);
  }
  ValidationResult ValidateDevice(const QVariantMap& parameters, const QVariantMap& state) const override {
    if (parameters.contains("modes") && !Advertises(parameters.value("modes").toList(), "value", state.value("value"))) {
      return std::unexpected(CapabilityMessages::tr("Устройство не поддерживает такой режим или сцену."));
    }
    return {};
  }
};
}
std::shared_ptr<const ICapabilityRules> Mode() { return std::make_shared<ModeRules>(); }
}
