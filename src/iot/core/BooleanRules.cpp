#include "Validation.h"

namespace Iot::Rules {
namespace {
class BooleanRules final : public ICapabilityRules {
public:
  BooleanRules(CapabilityType type, QString fixed_instance = {})
    : type_(type), fixed_instance_(std::move(fixed_instance)) {}
  CapabilityType Type() const override { return type_; }
  ValidationResult ValidateValue(const QVariantMap& state) const override {
    const auto instance = state.value("instance").toString();
    if (instance.isEmpty() || (!fixed_instance_.isEmpty() && instance != fixed_instance_)) {
      return std::unexpected(CapabilityMessages::tr("Укажите экземпляр умения; для on_off используется on."));
    }
    if (state.value("value").metaType().id() != QMetaType::Bool) {
      return std::unexpected(CapabilityMessages::tr("Логическое значение должно быть true или false."));
    }
    return {};
  }
  bool Supports(const CapabilityObject& capability, const QVariantMap& state) const override {
    return !fixed_instance_.isEmpty() || MatchesInstance(capability, state);
  }
  ValidationResult ValidateDevice(const QVariantMap&, const QVariantMap&) const override { return {}; }
private:
  CapabilityType type_;
  QString fixed_instance_;
};
}
std::shared_ptr<const ICapabilityRules> OnOff() { return std::make_shared<BooleanRules>(CapabilityType::OnOff, "on"); }
std::shared_ptr<const ICapabilityRules> Toggle() { return std::make_shared<BooleanRules>(CapabilityType::Toggle); }
}
