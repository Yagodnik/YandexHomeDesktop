#pragma once

#include "ICommand.h"

class AccountInfoCommand : public ICommand {
public:
  explicit AccountInfoCommand();

  void Execute(AppContext &app_ctx, const CommandContext &command_ctx) override;
};

