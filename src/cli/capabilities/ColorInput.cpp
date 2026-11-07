#include "Builtins.h"
#include "Input.h"
#include "ValueParsers.h"
#include "iot/core/CapabilityState.h"

namespace CliCapability {
namespace {
class ColorInput final : public Input {
public:
  ColorInput() : Input(Iot::Rules::Color()) {}
  StateResult Parse(const CliArguments& arguments) const override {
    static const QMap<QString, ValueParser> formats{
        {"rgb", RgbValue}, {"hsv", HsvValue}, {"temperature_k", NumberValue}, {"scene", TextValue}};
    const auto instance = arguments.Value("instance");
    const auto parse = formats.value(instance);
    // The shared rules report unsupported instances, including an empty one.
    if (!parse) {
      return Iot::State::Color(instance, arguments.Value("value"));
    }
    const auto value = parse(arguments.Value("value"));
    if (!value) {
      return std::unexpected(value.error());
    }
    return Iot::State::Color(instance, *value);
  }
};
} // namespace
std::shared_ptr<const ICapabilityInput> Color() {
  return std::make_shared<ColorInput>();
}
} // namespace CliCapability
