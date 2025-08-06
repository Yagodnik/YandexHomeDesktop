#pragma once

#include "ICommand.h"

class AccountInfoCommand : public ICommand {
  Q_OBJECT
public:
  explicit AccountInfoCommand(QObject *parent);

  void Execute(AppContext &app_ctx, const CommandContext &command_ctx) override;
};

