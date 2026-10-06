#include "Builtins.h"
#include "Input.h"
#include "ValueParsers.h"
#include "iot/core/CapabilityState.h"

namespace CliCapability {
namespace {
class BooleanInput final : public Input {
public:
  BooleanInput(std::shared_ptr<const Iot::ICapabilityRules> rules, QString default_instance = {})
    : Input(std::move(rules)), default_instance_(std::move(default_instance)) {}
  StateResult Parse(const CliArguments& arguments) const override {
    const auto value = BooleanValue(arguments.Value("value"));
    if (!value) { return std::unexpected(value.error()); }
    auto instance = arguments.Value("instance");
    if (instance.isEmpty()) { instance = default_instance_; }
    return Iot::State::Boolean(instance, value->toBool());
  }
private:
  QString default_instance_;
};
}
std::shared_ptr<const ICapabilityInput> OnOff() { return std::make_shared<BooleanInput>(Iot::Rules::OnOff(), "on"); }
std::shared_ptr<const ICapabilityInput> Toggle() { return std::make_shared<BooleanInput>(Iot::Rules::Toggle()); }
}
