#pragma once

#include <QGuiApplication>
#include <QString>

#include "AppContext.h"

struct CommandContext {
  bool pro_mode;
};

class ICommand {
public:
  explicit ICommand(const QString& command_name)
    : command_name_(command_name) {}

  virtual ~ICommand() = default;

  virtual void Execute(AppContext& app_ctx, const CommandContext& command_ctx) = 0;

protected:
  QString command_name_;
};
