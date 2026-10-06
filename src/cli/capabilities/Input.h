#pragma once
#include "cli/CapabilityCommands.h"

namespace CliCapability {
using StateResult = std::expected<QVariantMap, QString>;
class Input : public ICapabilityInput {
public:
  explicit Input(std::shared_ptr<const Iot::ICapabilityRules> rules) : rules_(std::move(rules)) {}
  std::shared_ptr<const Iot::ICapabilityRules> Rules() const override { return rules_; }
private:
  std::shared_ptr<const Iot::ICapabilityRules> rules_;
};
}
