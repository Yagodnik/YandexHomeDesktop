#include "ResetCommand.h"
#include <QDebug>

ResetCommand::ResetCommand() : ICommand("reset") {}

void ResetCommand::Execute(AppContext& app_ctx, const CommandContext& command_ctx) {
  QTextStream out(stdout);

  if (!command_ctx.pro_mode) {
    out << "Warning! --reset will reset all settings from app, specify "
                "--i-know-what-i-am-doing if know what you are doing" << Qt::endl;

    app_ctx.app_->quit();
    return;
  }

  out << "Resetting app settings..."<< Qt::endl;
  app_ctx.settings->Reset();

  out << "Logging out..." << Qt::endl;
  app_ctx.authorization_service->Logout();

  QObject::connect(
    app_ctx.authorization_service,
    &AuthorizationService::logoutFinished,
    [app_ctx] {
      QTextStream out(stdout);
      out << "Logged out!" << Qt::endl;
      out << "Quiting..." << Qt::endl;

      app_ctx.app_->quit();
    }
  );

  QObject::connect(
    app_ctx.authorization_service,
    &AuthorizationService::logoutFailed,
    [app_ctx](const QString &error) {
      QTextStream out(stdout);
      out << "Failed:" << error << Qt::endl;
      out << "Quiting..." << Qt::endl;

      app_ctx.app_->quit();
    }
  );
}
