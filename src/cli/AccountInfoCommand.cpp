#include "AccountInfoCommand.h"

#include <iostream>

AccountInfoCommand::AccountInfoCommand(QObject *parent) : ICommand("account-info", parent) {}

void AccountInfoCommand::Execute(AppContext &app_ctx, const CommandContext &command_ctx) {
  if (!app_ctx.authorization_service->IsAuthorized()) {
    std::cout << "Not authorized!" << std::endl;

    QGuiApplication::exit(0);
  }

  app_ctx.yandex_account->LoadData();

  connect(
    app_ctx.yandex_account,
    &YandexAccount::dataLoaded,
    [app_ctx]() {
      std::cout << "Account info: " << std::endl;
      std::cout << "Name: " << app_ctx.yandex_account->GetName().toStdString() << std::endl;
      std::cout << "Email: " << app_ctx.yandex_account->GetEmail().toStdString() << std::endl;

      QGuiApplication::exit(0);
    }
  );

  connect(
    app_ctx.yandex_account,
    &YandexAccount::dataLoadingFailed,
    [app_ctx]() {
      std::cout << "Account info loading failed!" << std::endl;

      QGuiApplication::exit(0);
    }
  );
}
