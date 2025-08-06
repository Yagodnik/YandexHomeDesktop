#include "AccountInfoCommand.h"

AccountInfoCommand::AccountInfoCommand() : ICommand("account-info") {}

void AccountInfoCommand::Execute(AppContext &app_ctx, const CommandContext &command_ctx) {
  if (!app_ctx.authorization_service->IsAuthorized()) {
    QTextStream out(stdout);
    out << "Not authorized!" << Qt::endl;
    app_ctx.app_->quit();
    return;
  }

  app_ctx.yandex_account->LoadData();

  QObject::connect(
    app_ctx.yandex_account,
    &YandexAccount::dataLoaded,
    [app_ctx]() {
      QTextStream out(stdout);
      out << "Account info: " << Qt::endl;
      out << "Name: " << app_ctx.yandex_account->GetName() << Qt::endl;

      app_ctx.app_->quit();
    }
  );

  QObject::connect(
    app_ctx.yandex_account,
    &YandexAccount::dataLoadingFailed,
    [app_ctx]() {
      QTextStream out(stdout);
      out << "Account info loading failed!" << Qt::endl;

      app_ctx.app_->quit();
    }
  );
}
