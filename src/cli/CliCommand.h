#pragma once

#include "CommandRegistry.h"

// Common invocation settings; command-specific arguments belong to ICommand.
struct CliCommand {
  Q_DECLARE_TR_FUNCTIONS(CliCommand)
public:
  std::shared_ptr<const ICommand> operation;
  bool json = false;
  int timeout_ms = 30000;
  static std::expected<CliCommand, QString> Parse(const QStringList& arguments,
    const CommandRegistry& registry = CommandRegistry::Builtin());
  static QString Help(const QString& program, const CommandRegistry& registry = CommandRegistry::Builtin());
};
