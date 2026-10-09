#pragma once

#include "yh/appcli_export.h"

class CliContext;

// A parsed command owns its arguments and executes against a shared context.
// New commands register a factory; parsing and execution never switch on enums.
class ICommand {
public:
  virtual ~ICommand() = default;
  [[nodiscard]] virtual bool RequiresAuthorization() const {
    return true;
  }
  [[nodiscard]] virtual bool UsesServices() const {
    return true;
  }
  virtual void Execute(CliContext& context) const = 0;
};
