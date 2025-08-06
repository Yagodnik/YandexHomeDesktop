#include "ResetCommand.h"

#include <iostream>
#include <QDebug>

ResetCommand::ResetCommand(QObject *parent) : ICommand("reset", parent) {}

void ResetCommand::Execute(AppContext& app_ctx, const CommandContext& command_ctx) {
  if (!command_ctx.pro_mode) {
    std::cout << "Warning! --reset will reset all settings from app, specify "
                "--i-know-what-i-am-doing if know what you are doing" << std::endl;

    QGuiApplication::exit(0);
    return;
  }

  connect(
    app_ctx.authorization_service,
    &AuthorizationService::logoutFinished,
    this,
    &ResetCommand::OnLogoutFinished);

  connect(
    app_ctx.authorization_service,
    &AuthorizationService::logoutFailed,
    this,
    &ResetCommand::OnLogoutFailed);

  std::cout << "Resetting app settings..." << std::endl;
  app_ctx.settings->Reset();

  std::cout << "Logging out..." << std::endl;
  app_ctx.authorization_service->Logout();
}

void ResetCommand::OnLogoutFinished() {
  std::cout << "Logged out!" << std::endl;
  std::cout << "Quiting..." << std::endl;

  QGuiApplication::exit(0);
}

void ResetCommand::OnLogoutFailed(const QString &error) {
  std::cout << "Failed: " << error.toStdString() << std::endl;
  std::cout << "Quiting..." << std::endl;

  QGuiApplication::exit(0);
}
