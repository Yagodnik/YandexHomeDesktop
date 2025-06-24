#include "CapabilityFactory.h"

#include <QColor>

CapabilityFactory::CapabilityFactory(QObject *parent)
  : QObject(parent) {}

QVariantMap CapabilityFactory::CreateOnOff(const CapabilityObject &info, bool value) {
  if (info.state["instance"] == "on") {
    qDebug() << "CapabilityFactory::CreateOnOff: on";
  } else {
    qDebug() << "CapabilityFactory::CreateOnOff: ???";
  }

  return {
    { "instance", "on" },
    { "value", value }
  };
}

QVariantMap CapabilityFactory::CreateRange(const CapabilityObject &info, int value) {
  const auto instance = info.state["instance"].toString();

  return {
    { "instance", instance },
    { "value", value }
  };
}

QVariantMap CapabilityFactory::CreateColorSetting(const CapabilityObject &info, const QColor &value) {
  qDebug() << "Color model: " << info.parameters["color_model"];
  qDebug() << info.parameters;
  qDebug() << "Color:" << value;

  if (info.parameters["color_model"] == "hsv") {
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

  if (info.parameters["color_model"] == "rgb") {
    uint32_t rgb_data = (value.red() << 16) | (value.green() << 8) | value.blue();

    return {
        { "instance", "rgb" },
        { "value", rgb_data }
    };
  }

  qDebug() << "CapabilityFactory::CreateColorSetting: Unknown color model";
  return {};
}

QVariantMap CapabilityFactory::CreateColorSetting(const CapabilityObject &info, int value) {
  Q_UNUSED(info);

  return {
  { "instance", "temperature_k" },
  { "value", value }
  };
}

QVariantMap CapabilityFactory::CreateColorSetting(const CapabilityObject &info, const QString &value) {
  Q_UNUSED(info);

  qDebug() << "Switching scene:" << value;

  return {
    { "instance", "scene" },
    { "value", value }
  };
}


