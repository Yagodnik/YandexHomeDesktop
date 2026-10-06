#pragma once

#include "api/model/Capabilites.h"

namespace Iot {
QString Instance(const QVariantMap& parameters);
bool MatchesInstance(const CapabilityObject& capability, const QVariantMap& state);
bool Advertises(const QVariantList& values, const QString& key, const QVariant& value);

class NumericLimits {
public:
  explicit NumericLimits(QVariantMap values) : values_(std::move(values)) {}
  double Min() const;
  double Max() const;
  double Precision() const;
  int IntMin() const;
  int IntMax() const;
  bool Contains(double value) const;
private:
  QVariantMap values_;
};

class RangeParameters {
public:
  explicit RangeParameters(QVariantMap values) : values_(std::move(values)) {}
  bool RandomAccess() const;
  bool RelativeOnly() const;
  NumericLimits Limits() const;
private:
  QVariantMap values_;
};

class ColorParameters {
public:
  explicit ColorParameters(QVariantMap values) : values_(std::move(values)) {}
  QString Model() const;
  bool HasColors() const;
  bool HasTemperature() const;
  bool HasScenes() const;
  NumericLimits Temperature() const;
  QVariantList Scenes() const;
  bool Supports(const QString& instance) const;
private:
  QVariantMap values_;
};
}
