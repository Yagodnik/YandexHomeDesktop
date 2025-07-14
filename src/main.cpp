#include <qcommandlineparser.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>
#include "api/YandexHomeApi.h"
#include "api/YandexAccount.h"
#include "auth/AuthorizationService.h"
#include "cli/CLI.h"
#include "iot/capabilities/ColorSettingCapability.h"
#include "models/ScenariosModel/ScenariosModel.h"
#include "models/DevicesModel//DevicesModel.h"
#include "models/DevicesModel/DevicesFilterModel.h"
#include "models/RoomsModel/RoomsModel.h"
#include "models/RoomsModel/RoomsFilterModel.h"
#include "models/DeviceModel/CapabilitiesModel.h"
#include "models/DeviceModel/PropertiesModel.h"
#include "models/ColorsModel/ColorsModel.h"
#include "models/ColorsModel/ColorModesModel.h"
#include "models/ColorsModel/ColorsFilterModel.h"
#include "models/ColorsModel/ColorModesFilterModel.h"
#include "models/HouseholdsModel/HouseholdsModel.h"
#include "models/ModesModel/ModesModel.h"
#include "models/ModesModel/ModesFilterModel.h"
#include "platform/PlatformService.h"
#include "utils/Router.h"
#include "utils/Themes.h"
#include "utils/ErrorCodes.h"
#include "utils/Settings.h"
#include "iot/capabilities/OnOffCapability.h"
#include "iot/capabilities/RangeCapability.h"
#include "iot/capabilities/ToggleCapability.h"
#include "iot/capabilities/ColorSettingCapability.h"
#include "iot/capabilities/ModesCapability.h"
#include "iot/properties/EventProperty.h"
#include "iot/properties/FloatProperty.h"
#include "models/DeviceModel/DeviceController.h"
#include "models/DeviceModel/DeviceDataModel.h"
#include "utils/IconsProvider.h"
#include "utils/UnitsList.h"

// TODO: Add logout button at error screen if some shit happened

void RegisterFonts(QGuiApplication &app) {
  const int id = QFontDatabase::addApplicationFont(":/fonts/Manrope-Regular.ttf");
  QFontDatabase::addApplicationFont(":/fonts/Manrope-Bold.ttf");

  const QString family = QFontDatabase::applicationFontFamilies(id).at(0);
  const QFont font(family);

  app.setFont(font);
}

void RegisterModels() {
  qmlRegisterType<DevicesFilterModel>("YandexHomeDesktop.Models", 1, 0, "DevicesFilterModel");
  qmlRegisterType<RoomsFilterModel>("YandexHomeDesktop.Models", 1, 0, "RoomsFilterModel");
  qmlRegisterType<ColorsFilterModel>("YandexHomeDesktop.Models", 1, 0, "ColorsFilterModel");
  qmlRegisterType<ColorModesFilterModel>("YandexHomeDesktop.Models", 1, 0, "ColorModesFilterModel");
  qmlRegisterType<ModesFilterModel>("YandexHomeDesktop.Models", 1, 0, "ModesFilterModel");
}

void RegisterCapabilities() {
  qmlRegisterType<OnOffCapability>("YandexHomeDesktop.Capabilities", 1, 0, "OnOff");
  qmlRegisterType<RangeCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Range");
  qmlRegisterType<ToggleCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Toggle");
  qmlRegisterType<ColorSettingCapability>("YandexHomeDesktop.Capabilities", 1, 0, "ColorSetting");
  qmlRegisterType<ModesCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Modes");
}

void RegisterProperties() {
  qmlRegisterType<FloatProperty>("YandexHomeDesktop.Properties", 1, 0, "Float");
  qmlRegisterType<EventProperty>("YandexHomeDesktop.Properties", 1, 0, "Event");
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    QCoreApplication app(argc, argv);
    CLI cli(&app);

    return app.exec();
  }

  QQuickStyle::setStyle("Basic");
  QGuiApplication app(argc, argv);

  RegisterFonts(app);

  QQmlApplicationEngine engine;

  const auto root_context = engine.rootContext();
  const auto authorization_service = new AuthorizationService(&app);
  const auto token_provider = [authorization_service] {
    const auto token = authorization_service->GetToken();
    if (!token.has_value()) {
      qWarning() << "AuthorizationService::GetToken: no token provided";
      QGuiApplication::quit();
    }

    return token.value();
  };
  const auto platform_service = new PlatformService(&app);
  const auto yandex_api = new YandexHomeApi(token_provider, &app);
  const auto yandex_account = new YandexAccount(token_provider, &app);
  const auto themes = new Themes(&app);
  const auto router = new Router(&app);
  const auto scenarios_model = new ScenariosModel(yandex_api, &app);
  const auto devices_model = new DevicesModel(yandex_api, &app);
  const auto rooms_model = new RoomsModel(yandex_api, &app);
  const auto device_controller = new DeviceController(yandex_api, &app);
  const auto households_model = new HouseholdsModel(yandex_api, &app);
  const auto error_codes = new ErrorCodes(&app);
  const auto color_model = new ColorsModel(&app);
  const auto color_modes_model = new ColorModesModel(&app);
  const auto modes_model = new ModesModel(&app);
  const auto settings = new Settings(&app);
  const auto titles_list = new TitlesProvider(":/data/instances.json", &app);
  const auto events_list = new TitlesProvider(":/data/propertiesEvents.json", &app);
  const auto units_list = new UnitsList(&app);
  const auto device_data_model = new DeviceDataModel(yandex_api, &app);
  const auto device_icons = new IconsProvider(":/data/deviceIcons.json", "devices", &app);
  const auto properties_icons = new IconsProvider(":/data/propertiesIcons.json", "properties", &app);
  // const auto capabilities_model = new CapabilitiesModel(yandex_api, &app);
  const auto capabilities_model = new CapabilitiesModel(device_controller, &app);
  const auto properties_model = new PropertiesModel(device_controller, &app);

  root_context->setContextProperty("platformService", platform_service);
  root_context->setContextProperty("authorizationService", authorization_service);
  root_context->setContextProperty("router", router);
  root_context->setContextProperty("scenariosModel", scenarios_model);
  root_context->setContextProperty("devicesModel", devices_model);
  root_context->setContextProperty("propertiesModel", properties_model);
  root_context->setContextProperty("deviceController", device_controller);
  root_context->setContextProperty("roomsModel", rooms_model);
  root_context->setContextProperty("yandexApi", yandex_api);
  root_context->setContextProperty("yandexAccount", yandex_account);
  root_context->setContextProperty("themes", themes);
  root_context->setContextProperty("capabilitiesModel", capabilities_model);
  root_context->setContextProperty("errorCodes", error_codes);
  root_context->setContextProperty("colorModel", color_model);
  root_context->setContextProperty("colorModesModel", color_modes_model);
  root_context->setContextProperty("modesModel", modes_model);
  root_context->setContextProperty("householdsModel", households_model);
  root_context->setContextProperty("settings", settings);
  root_context->setContextProperty("iotTitles", titles_list);
  root_context->setContextProperty("eventTitles", events_list);
  root_context->setContextProperty("unitsList", units_list);
  root_context->setContextProperty("deviceDataModel", device_data_model);
  root_context->setContextProperty("deviceIcons", device_icons);
  root_context->setContextProperty("propertiesIcons", properties_icons);

  RegisterModels();
  RegisterCapabilities();
  RegisterProperties();

  QObject::connect(
    &engine, &QQmlApplicationEngine::objectCreated,
    &app, [themes, settings, platform_service]() {
      if (settings->GetTrayModeEnabled()) {
        platform_service->ShowOnlyInTray();
      } else {
        platform_service->ShowAsApp();
      }

      themes->SetTheme(settings->GetCurrentTheme());
    }, Qt::QueuedConnection
  );

  QObject::connect(
    &engine, &QQmlApplicationEngine::objectCreationFailed,
    &app, []() {
      qCritical() << "QQmlApplicationEngine::objectCreationFailed";
      QCoreApplication::exit(-1);
    }, Qt::QueuedConnection
  );

  engine.loadFromModule("YandexHomeDesktop", "Main");

  return app.exec();
}
