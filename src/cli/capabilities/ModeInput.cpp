#include "Builtins.h"
#include "Input.h"
#include "iot/core/CapabilityState.h"

namespace CliCapability {
namespace {
class ModeInput final : public Input {
public:
  ModeInput() : Input(Iot::Rules::Mode()) {}
  StateResult Parse(const CliArguments& arguments) const override {
    return Iot::State::Mode(arguments.Value("instance"), arguments.Value("value"));
  }
};
} // namespace
std::shared_ptr<const ICapabilityInput> Mode() {
  return std::make_shared<ModeInput>();
}
} // namespace CliCapability
