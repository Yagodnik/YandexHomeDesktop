#include "GuiApp.h"

#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>

#include "api/YandexHomeApi.h"
#include "app/CliApp.h"
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
#include "utils/LogManager.h"
#include "utils/UnitsList.h"


GuiApp::GuiApp(AppContext& app_context, QObject *parent) :
  QObject(parent), app_context_(app_context)
{
  QQuickStyle::setStyle("Basic");

  const auto root_context = engine.rootContext();
  const auto themes = new Themes(app_context.app_);
  const auto router = new Router(app_context.app_);
  const auto scenarios_model = new ScenariosModel(app_context.yandex_api, app_context.app_);
  const auto devices_model = new DevicesModel(app_context.yandex_api, app_context.app_);
  const auto rooms_model = new RoomsModel(app_context.yandex_api, app_context.app_);
  const auto device_controller = new DeviceController(app_context.yandex_api, app_context.app_);
  const auto households_model = new HouseholdsModel(app_context.yandex_api, app_context.app_);
  const auto error_codes = new ErrorCodes(app_context.app_);
  const auto color_model = new ColorsModel(app_context.app_);
  const auto color_modes_model = new ColorModesModel(app_context.app_);
  const auto modes_model = new ModesModel(app_context.app_);
  const auto titles_list = new TitlesProvider(":/data/instances.json", app_context.app_);
  const auto events_list = new TitlesProvider(":/data/propertiesEvents.json", app_context.app_);
  const auto units_list = new UnitsList(app_context.app_);
  const auto device_data_model = new DeviceDataModel(device_controller, app_context.app_);
  const auto device_icons = new IconsProvider(":/data/deviceIcons.json", "devices", app_context.app_);
  const auto properties_icons = new IconsProvider(":/data/propertiesIcons.json", "properties", app_context.app_);
  const auto capabilities_model = new CapabilitiesModel(device_controller, app_context.app_);
  const auto properties_model = new PropertiesModel(device_controller, app_context.app_);

  root_context->setContextProperty("platformService", app_context.platform_service);
  root_context->setContextProperty("authorizationService", app_context.authorization_service);
  root_context->setContextProperty("router", router);
  root_context->setContextProperty("scenariosModel", scenarios_model);
  root_context->setContextProperty("devicesModel", devices_model);
  root_context->setContextProperty("propertiesModel", properties_model);
  root_context->setContextProperty("deviceController", device_controller);
  root_context->setContextProperty("roomsModel", rooms_model);
  root_context->setContextProperty("yandexApi", app_context.yandex_api);
  root_context->setContextProperty("yandexAccount", app_context.yandex_account);
  root_context->setContextProperty("themes", themes);
  root_context->setContextProperty("capabilitiesModel", capabilities_model);
  root_context->setContextProperty("errorCodes", error_codes);
  root_context->setContextProperty("colorModel", color_model);
  root_context->setContextProperty("colorModesModel", color_modes_model);
  root_context->setContextProperty("modesModel", modes_model);
  root_context->setContextProperty("householdsModel", households_model);
  root_context->setContextProperty("settings", app_context.settings);
  root_context->setContextProperty("iotTitles", titles_list);
  root_context->setContextProperty("eventTitles", events_list);
  root_context->setContextProperty("unitsList", units_list);
  root_context->setContextProperty("deviceDataModel", device_data_model);
  root_context->setContextProperty("deviceIcons", device_icons);
  root_context->setContextProperty("propertiesIcons", properties_icons);

  RegisterFonts();
  RegisterModels();
  RegisterCapabilities();
  RegisterProperties();

  QObject::connect(
  &engine, &QQmlApplicationEngine::objectCreated,
  app_context_.app_, [themes, app_context]() {
    if (app_context.settings->GetTrayModeEnabled()) {
      app_context.platform_service->ShowOnlyInTray();
    } else {
      app_context.platform_service->ShowAsApp();
    }

    themes->SetTheme(app_context.settings->GetCurrentTheme());
  }, Qt::QueuedConnection
);

  QObject::connect(
    &engine, &QQmlApplicationEngine::objectCreationFailed,
    app_context_.app_, [themes, app_context]() {
      qCritical() << "QQmlApplicationEngine::objectCreationFailed";
      QCoreApplication::exit(-1);
    }, Qt::QueuedConnection
  );
}

int GuiApp::Start() {
  engine.loadFromModule("YandexHomeDesktop", "Main");

  return app_context_.app_->exec();
}

void GuiApp::RegisterFonts() {
  const int id = QFontDatabase::addApplicationFont(":/fonts/Manrope-Regular.ttf");
  QFontDatabase::addApplicationFont(":/fonts/Manrope-Bold.ttf");

  const QString family = QFontDatabase::applicationFontFamilies(id).at(0);
  const QFont font(family);

  app_context_.app_->setFont(font);
}

void GuiApp::RegisterModels() {
  qmlRegisterType<DevicesFilterModel>("YandexHomeDesktop.Models", 1, 0, "DevicesFilterModel");
  qmlRegisterType<RoomsFilterModel>("YandexHomeDesktop.Models", 1, 0, "RoomsFilterModel");
  qmlRegisterType<ColorsFilterModel>("YandexHomeDesktop.Models", 1, 0, "ColorsFilterModel");
  qmlRegisterType<ColorModesFilterModel>("YandexHomeDesktop.Models", 1, 0, "ColorModesFilterModel");
  qmlRegisterType<ModesFilterModel>("YandexHomeDesktop.Models", 1, 0, "ModesFilterModel");
}

void GuiApp::RegisterCapabilities() {
  qmlRegisterType<OnOffCapability>("YandexHomeDesktop.Capabilities", 1, 0, "OnOff");
  qmlRegisterType<RangeCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Range");
  qmlRegisterType<ToggleCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Toggle");
  qmlRegisterType<ColorSettingCapability>("YandexHomeDesktop.Capabilities", 1, 0, "ColorSetting");
  qmlRegisterType<ModesCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Modes");
}

void GuiApp::RegisterProperties() {
  qmlRegisterType<FloatProperty>("YandexHomeDesktop.Properties", 1, 0, "Float");
  qmlRegisterType<EventProperty>("YandexHomeDesktop.Properties", 1, 0, "Event");
}
