#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "api/YandexHomeApi.h"
#include "api/model/UserInfo.h"
#include "auth/AuthorizationService.h"
#include "models/ScenariosModel.h"
#include "Router.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  const auto root_context = engine.rootContext();
  const auto authorization_service = new AuthorizationService();
  const auto yandex_api = new YandexHomeApi([authorization_service] {
    return authorization_service->GetToken();
  });

  root_context->setContextProperty("authorizationService", authorization_service);
  root_context->setContextProperty("router", new Router());
  root_context->setContextProperty("scenariosModel", new ScenariosModel(yandex_api));
  root_context->setContextProperty("yandex_api", yandex_api);

  QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreationFailed,
    &app,
    []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);

  engine.loadFromModule("YandexHomeDesktop", "Main");

  return app.exec();
}

