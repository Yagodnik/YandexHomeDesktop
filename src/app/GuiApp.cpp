#include "GuiApp.h"

#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>

#include "api/YandexHomeApi.h"
#include "iot/capabilities/ColorSettingCapability.h"
#include "models/ScenariosModel/ScenariosViewModel.h"
#include "models/HomeViewModel.h"
#include "models/AuthorizationModel.h"
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
#include "models/DeviceModel/DeviceViewModel.h"
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
  const auto scenarios_view_model = new ScenariosViewModel(app_context.scenario_service, app_context.app_);
  const auto home_view_model = new HomeViewModel(app_context.home_service, app_context.settings,
    app_context.yandex_account, app_context.app_);
  const auto authorization_model = new AuthorizationModel(app_context.authorization_service, app_context.app_);
  const auto device_view_model = new DeviceViewModel(app_context.device_service, app_context.app_);
  const auto error_codes = new ErrorCodes(app_context.app_);
  const auto color_model = new ColorsModel(app_context.app_);
  const auto color_modes_model = new ColorModesModel(app_context.app_);
  const auto modes_model = new ModesModel(app_context.app_);
  const auto titles_list = new TitlesProvider(":/data/instances.json", "DataInstances", app_context.app_);
  const auto events_list = new TitlesProvider(":/data/propertiesEvents.json", "DataEvents", app_context.app_);
  const auto units_list = new UnitsList(app_context.app_);
  const auto device_data_model = device_view_model->GetDeviceData();
  const auto device_icons = new IconsProvider(":/data/deviceIcons.json", "devices", app_context.app_);
  const auto properties_icons = new IconsProvider(":/data/propertiesIcons.json", "properties", app_context.app_);
  const auto capabilities_model = device_view_model->GetCapabilities();
  const auto properties_model = device_view_model->GetProperties();

  root_context->setContextProperty("platformService", app_context.platform_service);
  root_context->setContextProperty("authorizationService", authorization_model);
  root_context->setContextProperty("router", router);
  root_context->setContextProperty("scenariosViewModel", scenarios_view_model);
  root_context->setContextProperty("homeViewModel", home_view_model);
  root_context->setContextProperty("propertiesModel", properties_model);
  root_context->setContextProperty("deviceViewModel", device_view_model);
  root_context->setContextProperty("deviceController", device_view_model);
  root_context->setContextProperty("yandexAccount", app_context.yandex_account);
  root_context->setContextProperty("themes", themes);
  root_context->setContextProperty("capabilitiesModel", capabilities_model);
  root_context->setContextProperty("errorCodes", error_codes);
  root_context->setContextProperty("colorModel", color_model);
  root_context->setContextProperty("colorModesModel", color_modes_model);
  root_context->setContextProperty("modesModel", modes_model);
  root_context->setContextProperty("settings", app_context.settings);
  root_context->setContextProperty("iotTitles", titles_list);
  root_context->setContextProperty("eventTitles", events_list);
  root_context->setContextProperty("unitsList", units_list);
  root_context->setContextProperty("deviceDataModel", device_data_model);
  root_context->setContextProperty("deviceIcons", device_icons);
  QObject::connect(app_context.authorization_service, &IAuthorizationService::logout,
    device_view_model, &DeviceViewModel::ResetSession);
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
  qmlRegisterUncreatableType<DeviceViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "DeviceViewModel", "Supplied by the application");
  qmlRegisterUncreatableType<CapabilitiesModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "CapabilitiesModel", "Supplied by the view model");
  qmlRegisterUncreatableType<PropertiesModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "PropertiesModel", "Supplied by the view model");
  qmlRegisterUncreatableType<DeviceDataModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "DeviceDataModel", "Supplied by the view model");
  qmlRegisterUncreatableType<HomeViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "HomeViewModel", "Supplied by the application");
  qmlRegisterUncreatableType<DevicesModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "DevicesModel", "Supplied by the application");
  qmlRegisterUncreatableType<RoomsModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "RoomsModel", "Supplied by the application");
  qmlRegisterUncreatableType<HouseholdsModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "HouseholdsModel", "Supplied by the application");
  qmlRegisterUncreatableType<ScenariosViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "ScenariosViewModel", "ScenariosViewModel is supplied by the application");
  qmlRegisterUncreatableType<ScenariosModel>("YandexHomeDesktop.ViewModels", 1, 0,
    "ScenariosModel", "ScenariosModel is supplied by ScenariosViewModel");
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
