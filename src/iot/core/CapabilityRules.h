#pragma once

#include <QCoreApplication>
#include <expected>
#include <memory>
#include "CapabilityParameters.h"

class CapabilityMessages { Q_DECLARE_TR_FUNCTIONS(CapabilityMessages) };

namespace Iot {
using ValidationResult = std::expected<void, QString>;

// Domain rules use typed state and device metadata, never CLI options or QML.
class ICapabilityRules {
public:
  virtual ~ICapabilityRules() = default;
  virtual CapabilityType Type() const = 0;
  virtual bool SupportsRelative() const { return false; }
  virtual ValidationResult ValidateValue(const QVariantMap& state) const = 0;
  bool Matches(const CapabilityObject& capability, const QVariantMap& state) const {
    return capability.type == Type() && Supports(capability, state);
  }
  virtual ValidationResult ValidateDevice(const QVariantMap& parameters, const QVariantMap& state) const = 0;
protected:
  virtual bool Supports(const CapabilityObject& capability, const QVariantMap& state) const = 0;
};

ValidationResult ValidateInput(const ICapabilityRules& rules, const QVariantMap& state);
namespace Rules {
std::shared_ptr<const ICapabilityRules> OnOff();
std::shared_ptr<const ICapabilityRules> Toggle();
std::shared_ptr<const ICapabilityRules> Range();
std::shared_ptr<const ICapabilityRules> Mode();
std::shared_ptr<const ICapabilityRules> Color();
}
}
