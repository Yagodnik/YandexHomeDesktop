#pragma once
#include "ICommand.h"

class ResetCommand final : public ICommand {
  Q_OBJECT
public:
  explicit ResetCommand(QObject *parent);

  void Execute(AppContext& app_ctx, const CommandContext& command_ctx) override;

private slots:
  void OnLogoutFinished();
  void OnLogoutFailed(const QString &error);
};
