#include "CapabilityParameters.h"
#include <algorithm>

namespace Iot {
QString Instance(const QVariantMap& parameters) {
  return parameters.value("instance").toString();
}
bool MatchesInstance(const CapabilityObject& capability, const QVariantMap& state) {
  return capability.parameters.value("instance", capability.state.value("instance")) == state.value("instance");
}
bool Advertises(const QVariantList& values, const QString& key, const QVariant& value) {
  return std::any_of(values.begin(), values.end(), [&](const QVariant& entry) {
    return entry.toMap().value(key) == value;
  });
}
double NumericLimits::Min() const { return values_.value("min").toDouble(); }
double NumericLimits::Max() const { return values_.value("max").toDouble(); }
double NumericLimits::Precision() const { return values_.value("precision").toDouble(); }
int NumericLimits::IntMin() const { return values_.value("min").toInt(); }
int NumericLimits::IntMax() const { return values_.value("max").toInt(); }
bool NumericLimits::Contains(double value) const {
  if (values_.contains("min") && value < Min()) { return false; }
  if (values_.contains("max") && value > Max()) { return false; }
  return true;
}
bool RangeParameters::RandomAccess() const { return values_.value("random_access", false).toBool(); }
bool RangeParameters::RelativeOnly() const { return values_.contains("random_access") && !RandomAccess(); }
NumericLimits RangeParameters::Limits() const { return NumericLimits(values_.value("range").toMap()); }
QString ColorParameters::Model() const { return values_.value("color_model").toString(); }
bool ColorParameters::HasColors() const { return values_.contains("color_model"); }
bool ColorParameters::HasTemperature() const { return values_.contains("temperature_k"); }
bool ColorParameters::HasScenes() const { return values_.contains("color_scene"); }
NumericLimits ColorParameters::Temperature() const { return NumericLimits(values_.value("temperature_k").toMap()); }
QVariantList ColorParameters::Scenes() const { return values_.value("color_scene").toMap().value("scenes").toList(); }
bool ColorParameters::Supports(const QString& instance) const {
  if (instance == "rgb" || instance == "hsv") { return Model() == instance; }
  if (instance == "temperature_k") { return HasTemperature(); }
  if (instance == "scene") { return HasScenes(); }
  return false;
}
}
