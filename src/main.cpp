#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "api/YandexHomeApi.h"
#include "api/YandexAccount.h"
#include "api/CapabilityFactory.h"
#include "auth/AuthorizationService.h"
#include "models/ScenariosModel/ScenariosModel.h"
#include "models/DevicesModel//DevicesModel.h"
#include "models/DevicesModel/DevicesFilterModel.h"
#include "models/RoomsModel/RoomsModel.h"
#include "models/RoomsModel/RoomsFilterModel.h"
#include "models/DeviceModel/DeviceModel.h"
#include "models/ColorsModel/ColorsModel.h"
#include "models/ModesModel/ModesModel.h"
#include "models/ColorsModel/ColorsFilterModel.h"
#include "models/ModesModel/ModesFilterModel.h"
#include "platform/PlatformService.h"
#include "utils/Router.h"
#include "utils/Themes.h"
#include "utils/ErrorCodes.h"
#include <QQuickStyle>

int main(int argc, char *argv[]) {
  QQuickStyle::setStyle("Basic");

  QGuiApplication app(argc, argv);
  // app.setQuitOnLastWindowClosed(false);

  QQmlApplicationEngine engine;

  const auto root_context = engine.rootContext();
  const auto authorization_service = new AuthorizationService(&app);
  const auto token_provider = [authorization_service] {
    return authorization_service->GetToken();
  };
  const auto platform_service = new PlatformService(&app);
  const auto yandex_api = new YandexHomeApi(token_provider, &app);
  const auto yandex_account = new YandexAccount(token_provider, &app);
  const auto themes = new Themes(&app);
  const auto router = new Router(&app);
  const auto scenarios_model = new ScenariosModel(yandex_api, &app);
  const auto devices_model = new DevicesModel(yandex_api, &app);
  const auto rooms_model = new RoomsModel(yandex_api, &app);
  const auto device_model = new DeviceModel(yandex_api, &app);
  const auto capability_factory = new CapabilityFactory(&app);
  const auto error_codes = new ErrorCodes(&app);
  const auto color_model = new ColorsModel(&app);
  const auto modes_model = new ModesModel(&app);

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
  root_context->setContextProperty("capabilityFactory", capability_factory);
  root_context->setContextProperty("errorCodes", error_codes);
  root_context->setContextProperty("colorModel", color_model);
  root_context->setContextProperty("modesModel", modes_model);

  qmlRegisterType<DevicesFilterModel>("YandexHomeDesktop.Models", 1, 0, "DevicesFilterModel");
  qmlRegisterType<RoomsFilterModel>("YandexHomeDesktop.Models", 1, 0, "RoomsFilterModel");
  qmlRegisterType<ColorsFilterModel>("YandexHomeDesktop.Models", 1, 0, "ColorsFilterModel");
  qmlRegisterType<ModesFilterModel>("YandexHomeDesktop.Models", 1, 0, "ModesFilterModel");

  QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreationFailed,
    &app,
    []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);

  engine.loadFromModule("YandexHomeDesktop", "Main");

  return app.exec();
}
