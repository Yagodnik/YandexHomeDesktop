#include "CapabilityState.h"

namespace {
QVariantMap MakeState(const QString& instance, const QVariant& value) {
  return {{"instance", instance}, {"value", value}};
}
}

namespace Iot::State {
QVariantMap Boolean(const QString& instance, bool value) { return MakeState(instance, value); }
QVariantMap OnOff(bool value) { return Boolean("on", value); }
QVariantMap WithRelative(QVariantMap state) {
  state["relative"] = true;
  return state;
}
QVariantMap Toggle(const QString& instance, bool value) { return Boolean(instance, value); }
QVariantMap Range(const QString& instance, double value) { return MakeState(instance, value); }
QVariantMap RelativeRange(const QString& instance, double delta) {
  return WithRelative(Range(instance, delta));
}
QVariantMap Mode(const QString& instance, const QString& value) { return MakeState(instance, value); }
QVariantMap Color(const QString& instance, const QVariant& value) { return MakeState(instance, value); }
QVariantMap Rgb(const QVariant& value) { return Color("rgb", value); }
QVariantMap Hsv(const QVariantMap& value) { return Color("hsv", value); }
QVariantMap Temperature(const QVariant& value) { return Color("temperature_k", value); }
QVariantMap Scene(const QString& value) { return Color("scene", value); }
quint32 PackRgb(int red, int green, int blue) { return (red << 16) | (green << 8) | blue; }
QVariantMap HsvComponents(int hue, int saturation, int value) {
  return {{"h", hue}, {"s", saturation}, {"v", value}};
}
}
