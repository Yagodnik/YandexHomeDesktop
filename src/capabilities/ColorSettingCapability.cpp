#include "ColorSettingCapability.h"
#include <QColor>

ColorSettingCapability::ColorSettingCapability(QObject *parent) : ICapability(parent) {}

void ColorSettingCapability::SetValue(const QVariant &value) {
  if (GetValue() == value) {
    return;
  }

  state_["value"] = value;

  emit valueChanged();
  emit stateChanged();
}

QVariant ColorSettingCapability::GetValue() const {
  if (state_.isEmpty()) {
    return false;
  }

  return state_["value"];
}

QVariantMap ColorSettingCapability::Create(const QColor &value) {
  if (parameters_["color_model"] == "hsv") {
    QVariantMap hsv_data = {
      { "h", static_cast<int>(value.hue()) },
      { "s", static_cast<int>(value.saturation() / 255.0 * 100) },
      { "v", static_cast<int>(value.value() / 255.0 * 100) }
    };

    qDebug() << "HSV: " << hsv_data;

    return {
        { "instance", "hsv" },
        { "value", hsv_data }
    };
  }

  if (parameters_["color_model"] == "rgb") {
    uint32_t rgb_data = (value.red() << 16) | (value.green() << 8) | value.blue();

    return {
          { "instance", "rgb" },
          { "value", rgb_data }
    };
  }

  qDebug() << "CapabilityFactory::CreateColorSetting: Unknown color model";
  return {};
}

QVariantMap ColorSettingCapability::Create(const int value) {
  return {
    { "instance", "temperature_k" },
    { "value", value }
  };
}

QVariantMap ColorSettingCapability::Create(const QString &value) {
  return {
    { "instance", "scene" },
    { "value", value }
  };
}

int ColorSettingCapability::GetTemperatureMin() const {
  const auto temperature_k = parameters_["temperature_k"].toMap();
  return temperature_k["min"].toInt();
}

int ColorSettingCapability::GetTemperatureMax() const {
  const auto temperature_k = parameters_["temperature_k"].toMap();
  return temperature_k["max"].toInt();
}

QVariantList ColorSettingCapability::GetAvailableScenes() const {
  const auto color_scene = parameters_["color_scene"].toMap();
  return color_scene["scenes"].toList();
}
