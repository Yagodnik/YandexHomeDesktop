#include "ColorSettingCapability.h"
#include <QColor>
#include "iot/core/CapabilityState.h"
#include "iot/core/CapabilityParameters.h"

ColorSettingCapability::ColorSettingCapability(QObject *parent) : IotObject("base", parent) {}

void ColorSettingCapability::SetValue(const QVariant &value) {
  if (GetValue() == value) {
    return;
  }

  SetStateValue(value);
}

QVariant ColorSettingCapability::GetValue() const {
  if (state_.isEmpty()) {
    return false;
  }

  return state_["value"];
}

QVariantMap ColorSettingCapability::Create(const QColor &value) {
  const auto model = Iot::ColorParameters(parameters_).Model();
  if (model == "hsv") {
    // Preserve the desktop's integer truncation and QColor's undefined hue (-1).
    const auto hsv = Iot::State::HsvComponents(value.hue(),
      static_cast<int>(value.saturation() / 255.0 * 100),
      static_cast<int>(value.value() / 255.0 * 100));
    qInfo() << "ColorSettingCapability: Setting HSV:" << hsv;
    return Iot::State::Hsv(hsv);
  }
  if (model == "rgb") {
    const auto rgb = Iot::State::PackRgb(value.red(), value.green(), value.blue());
    qInfo() << "ColorSettingCapability: Setting RGB:" << rgb;
    return Iot::State::Rgb(rgb);
  }
  qWarning() << "CapabilityFactory::CreateColorSetting: Unknown color model:" << parameters_["color_model"];
  return {};
}

QVariantMap ColorSettingCapability::Create(const int value) {
  return Iot::State::Temperature(value);
}

QVariantMap ColorSettingCapability::Create(const QString &value) {
  return Iot::State::Scene(value);
}

int ColorSettingCapability::GetTemperatureMin() const {
  return Iot::ColorParameters(parameters_).Temperature().IntMin();
}

int ColorSettingCapability::GetTemperatureMax() const {
  return Iot::ColorParameters(parameters_).Temperature().IntMax();
}

QVariantList ColorSettingCapability::GetAvailableScenes() const {
  return Iot::ColorParameters(parameters_).Scenes();
}

bool ColorSettingCapability::GetSupportsColors() const {
  return Iot::ColorParameters(parameters_).HasColors();
}

bool ColorSettingCapability::GetSupportsTemperature() const {
  return Iot::ColorParameters(parameters_).HasTemperature();
}

void ColorSettingCapability::SetParameters(const QVariantMap &parameters) {
  IotObject::SetParameters(parameters);

  emit temperatureMinChanged();
  emit temperatureMaxChanged();
  emit availableScenesChanged();
  emit supportsColorsChanged();
  emit supportsTemperatureChanged();
}

void ColorSettingCapability::SetTemperatureMin(int value) {
  const auto temperature_k = parameters_["temperature_k"].toMap();
  temperature_k["min"] = value;
  parameters_["temperature_k"] = temperature_k;

  emit temperatureMinChanged();
}

void ColorSettingCapability::SetTemperatureMax(int value) {
  const auto temperature_k = parameters_["temperature_k"].toMap();
  temperature_k["max"] = value;
  parameters_["temperature_k"] = temperature_k;

  emit temperatureMinChanged();
}

void ColorSettingCapability::SetAvailableScenes(const QVariantList &value) {
  const auto temperature_k = parameters_["color_scene"].toMap();
  temperature_k["scenes"] = value;
  parameters_["color_scene"] = temperature_k;

  emit availableScenesChanged();
}
