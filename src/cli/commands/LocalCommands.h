#pragma once

#include "yh/appcli_export.h"
#include "cli/CommandRegistry.h"

class LocalCommands {
  Q_DECLARE_TR_FUNCTIONS(LocalCommands)
};
class APPCLI_EXPORT HelpCommand final : public ICommand {
public:
  explicit HelpCommand(QString text) : text_(std::move(text)) {}
  bool RequiresAuthorization() const override {
    return false;
  }
  bool UsesServices() const override {
    return false;
  }
  void Execute(CliContext& context) const override;

private:
  QString text_;
};
APPCLI_EXPORT void RegisterLocalCommands(CommandRegistry& registry);
