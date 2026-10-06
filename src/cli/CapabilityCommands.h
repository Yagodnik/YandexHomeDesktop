#pragma once

#include "CliArguments.h"
#include "iot/core/CapabilityRules.h"

// CLI adapters convert text only. Matching and validation belong to IoT rules.
class ICapabilityInput {
public:
  virtual ~ICapabilityInput() = default;
  virtual std::shared_ptr<const Iot::ICapabilityRules> Rules() const = 0;
  virtual std::expected<QVariantMap, QString> Parse(const CliArguments& arguments) const = 0;
};

struct CapabilityAction {
  std::shared_ptr<const Iot::ICapabilityRules> rules;
  QVariantMap state;
};

class CapabilityCommands {
  Q_DECLARE_TR_FUNCTIONS(CapabilityCommands)
public:
  void Register(const QString& name, std::shared_ptr<const ICapabilityInput> input);
  std::expected<CapabilityAction, QString> Parse(const CliArguments& arguments) const;
  static CapabilityCommands Builtin();
private:
  QMap<QString, std::shared_ptr<const ICapabilityInput>> inputs_;
};
