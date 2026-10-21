#include "Validation.h"
#include <QMap>
#include <cmath>
#include <limits>

namespace Iot::Rules {
namespace {
ValidationResult InvalidColor() {
  return std::unexpected(CapabilityMessages::tr("Цвет: rgb — целое 0–16777215; temperature_k — положительное целое; также доступны hsv и scene."));
}
ValidationResult IntegerColor(const QVariant& value, double min, double max) {
  if (!IsFiniteNumber(value)) {
    return std::unexpected(CapabilityMessages::tr("Значение должно быть конечным числом."));
  }
  const auto number = value.toDouble();
  if (number != std::floor(number) || number < min || number > max) { return InvalidColor(); }
  return {};
}
ValidationResult ValidateRgb(const QVariant& value) { return IntegerColor(value, 0, 0xffffff); }
ValidationResult ValidateTemperature(const QVariant& value) {
  return IntegerColor(value, 1, std::numeric_limits<double>::max());
}
ValidationResult ValidateScene(const QVariant& value) {
  if (value.toString().isEmpty()) {
    return std::unexpected(CapabilityMessages::tr("Режим или сцена не должны быть пустыми."));
  }
  return {};
}
bool HsvComponent(const QVariantMap& hsv, const QString& component, double max) {
  const auto value = hsv.value(component);
  return IsFiniteNumber(value) && value.toDouble() >= 0 && value.toDouble() <= max;
}
ValidationResult ValidateHsv(const QVariant& value) {
  if (value.metaType().id() != QMetaType::QVariantMap) {
    return std::unexpected(CapabilityMessages::tr("HSV должен быть JSON-объектом с h, s и v."));
  }
  const auto hsv = value.toMap();
  if (hsv.size() != 3 || !HsvComponent(hsv, "h", 360) || !HsvComponent(hsv, "s", 100) || !HsvComponent(hsv, "v", 100)) {
    return std::unexpected(CapabilityMessages::tr("HSV: h должен быть 0–360; s и v — 0–100."));
  }
  return {};
}

class ColorRules final : public ICapabilityRules {
public:
  CapabilityType Type() const override { return CapabilityType::ColorSetting; }
  ValidationResult ValidateValue(const QVariantMap& state) const override {
    using Validator = ValidationResult (*)(const QVariant&);
    static const QMap<QString, Validator> validators{
      {"rgb", ValidateRgb}, {"hsv", ValidateHsv},
      {"temperature_k", ValidateTemperature}, {"scene", ValidateScene}
    };
    const auto validate = validators.value(state.value("instance").toString());
    if (!validate) { return InvalidColor(); }
    return validate(state.value("value"));
  }
  bool Supports(const CapabilityObject& capability, const QVariantMap& state) const override {
    return ColorParameters(capability.parameters).Supports(state.value("instance").toString());
  }
  ValidationResult ValidateDevice(const QVariantMap& parameters, const QVariantMap& state) const override {
    const ColorParameters color(parameters);
    const auto instance = state.value("instance").toString();
    if (instance == "temperature_k") { return CheckLimits(color.Temperature(), state.value("value").toDouble()); }
    if (instance == "scene" && !Advertises(color.Scenes(), "id", state.value("value"))) {
      return std::unexpected(CapabilityMessages::tr("Устройство не поддерживает такой режим или сцену."));
    }
    return {};
  }
};
}
std::shared_ptr<const ICapabilityRules> Color() { return std::make_shared<ColorRules>(); }
}
