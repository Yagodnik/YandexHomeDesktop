#include "Builtins.h"
#include "Input.h"
#include "ValueParsers.h"
#include "iot/core/CapabilityState.h"

namespace CliCapability {
namespace {
class RangeInput final : public Input {
public:
  RangeInput() : Input(Iot::Rules::Range()) {}
  StateResult Parse(const CliArguments& arguments) const override {
    const auto value = NumberValue(arguments.Value("value"));
    if (!value) { return std::unexpected(value.error()); }
    return Iot::State::Range(arguments.Value("instance"), value->toDouble());
  }
};
}
std::shared_ptr<const ICapabilityInput> Range() { return std::make_shared<RangeInput>(); }
}
