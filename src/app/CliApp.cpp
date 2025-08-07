#include "CliApp.h"

#include "cli/CLI.h"

CliApp::CliApp(AppContext &app_context, QObject *parent) :
  QObject(parent), app_context_(app_context)
{
  connect(
    app_context_.authorization_service,
    &AuthorizationService::authorized,
    this,
    &CliApp::onAuthorized);

  connect(
    app_context_.authorization_service,
    &AuthorizationService::authorizationFailed,
    this,
    &CliApp::onAuthorizationFailed);

  connect(
    app_context_.authorization_service,
    &AuthorizationService::authorizationCanceled,
    this,
    &CliApp::onAuthorizationFailed);

  connect(
    app_context_.authorization_service,
    &AuthorizationService::unauthorized,
    this,
    &CliApp::onAuthorized);

  // We need to hide icon when we run in CLI mode
  app_context_.platform_service->ShowOnlyInTray();
  app_context_.authorization_service->AttemptLocalAuthorization();
}

int CliApp::Start() {
  return app_context_.app_->exec();
}

void CliApp::onAuthorized() {
  CLI cli(app_context_, app_context_.yandex_api);
}

void CliApp::onUnauthorized() {
  qDebug() << "Not authorized!";

  app_context_.app_->quit();
}

void CliApp::onAuthorizationFailed() {
  qDebug() << "Authorization failed!";

  app_context_.app_->quit();
}
