#pragma once
#include "ICommand.h"

class ResetCommand final : public ICommand {
public:
  explicit ResetCommand();

  void Execute(AppContext& app_ctx, const CommandContext& command_ctx) override;
};
