#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "api/YandexHomeApi.h"
#include "api/YandexAccount.h"
#include "auth/AuthorizationService.h"
#include "models/ScenariosModel.h"
#include "models/DevicesModel.h"
#include "models/DevicesFilterModel.h"
#include "models/RoomsModel.h"
#include "models/RoomsFilterModel.h"
#include "models/DeviceModel.h"
#include "platform/PlatformService.h"
#include "Router.h"
#include "Themes.h"
#include <QQuickStyle>

int main(int argc, char *argv[]) {
  QQuickStyle::setStyle("Basic");

  QGuiApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);

  QQmlApplicationEngine engine;

  const auto root_context = engine.rootContext();
  const auto authorization_service = new AuthorizationService();
  const auto token_provider = [authorization_service] {
    return authorization_service->GetToken();
  };
  const auto platform_service = new PlatformService();
  const auto yandex_api = new YandexHomeApi(token_provider);
  const auto yandex_account = new YandexAccount(token_provider);
  const auto themes = new Themes();
  const auto router = new Router();
  const auto scenarios_model = new ScenariosModel(yandex_api);
  const auto devices_model = new DevicesModel(yandex_api);
  const auto rooms_model = new RoomsModel(yandex_api);
  const auto device_model = new DeviceModel(yandex_api);

  root_context->setContextProperty("platformService", platform_service);
  root_context->setContextProperty("authorizationService", authorization_service);
  root_context->setContextProperty("router", router);
  root_context->setContextProperty("scenariosModel", scenarios_model);
  root_context->setContextProperty("devicesModel", devices_model);
  root_context->setContextProperty("roomsModel", rooms_model);
  root_context->setContextProperty("yandexApi", yandex_api);
  root_context->setContextProperty("yandexAccount", yandex_account);
  root_context->setContextProperty("themes", themes);
  root_context->setContextProperty("deviceModel", device_model);

  qmlRegisterType<DevicesFilterModel>("YandexHomeDesktop.Models", 1, 0, "DevicesFilterModel");
  qmlRegisterType<RoomsFilterModel>("YandexHomeDesktop.Models", 1, 0, "RoomsFilterModel");


  QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreationFailed,
    &app,
    []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);

  engine.loadFromModule("YandexHomeDesktop", "Main");

  return app.exec();
}
