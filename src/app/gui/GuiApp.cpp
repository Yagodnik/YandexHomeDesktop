#include "GuiApp.h"

#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>
#include <QEventLoop>
#include <QTimer>
#include "app/shortcuts/ShortcutConfiguration.h"
#include "models/ShortcutViewModel.h"

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
#include "models/DeviceModel/CameraSnapshotModel.h"
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
#include "app/rest/RestConfiguration.h"
#include "models/RestViewModel.h"
#ifdef YH_MCP_ENABLED
#include "app/mcp/McpConfiguration.h"
#include "app/mcp/McpGuiHost.h"
#include "models/McpViewModel.h"
#endif
#include "iot/capabilities/OnOffCapability.h"
#include "iot/capabilities/RangeCapability.h"
#include "iot/capabilities/ToggleCapability.h"
#include "iot/capabilities/ColorSettingCapability.h"
#include "iot/capabilities/ModesCapability.h"
#include "iot/properties/EventProperty.h"
#include "iot/properties/FloatProperty.h"
#include "models/DeviceModel/DeviceViewModel.h"
#include "models/KeybindSettingsModel.h"
#include "models/SettingsTransferModel.h"
#include "models/DeviceModel/DeviceDataModel.h"
#include "utils/IconsProvider.h"
#include "utils/LogManager.h"
#include "utils/UnitsList.h"

GuiApp::GuiApp(AppContext& app_context, QObject* parent)
    : QObject(parent), app_context_(app_context) {
  QQuickStyle::setStyle("Basic");

  const auto root_context = engine.rootContext();
#ifdef YH_MCP_ENABLED
  const auto mcp_configuration = MakeMcpConfiguration(app_context.startup_options);
  auto* mcp_host = new McpGuiHost(mcp_configuration.gui_endpoint, app_context.home_service,
                                  app_context.device_service, app_context.scenario_service,
                                  [auth = app_context.authorization_service] { return auth->IsAuthorized(); },
                                  [settings = app_context.settings] { return settings->GetMcpPort(); },
                                  [settings = app_context.settings](quint16 port) { settings->SetMcpPort(port); },
                                  app_context.app_);
  mcp_host->Start();
  auto* mcp_model = new McpViewModel(mcp_configuration.worker, app_context.settings, app_context.app_);
  root_context->setContextProperty("mcpViewModel", mcp_model);
  connect(app_context.authorization_service, &IAuthorizationService::logout, mcp_model,
          [mcp_model, mcp_host] { mcp_host->Reset(); mcp_model->SetEnabled(false); });
#else
  root_context->setContextProperty("mcpViewModel", QVariant{});
#endif
  root_context->setContextProperty(
      "settingsTransferModel",
      new SettingsTransferModel(new SettingsTransferService(app_context.settings, app_context.app_),
                                app_context.yandex_account, app_context.app_));
  shortcuts_ = new ShortcutControlService(
      app_context.settings, ShortcutConfiguration(app_context.startup_options), app_context.app_);
  root_context->setContextProperty("shortcutViewModel",
                                   new ShortcutViewModel(shortcuts_, app_context.app_));
  const auto keybind_settings =
      new KeybindSettingsModel(app_context.settings, app_context.yandex_account, app_context.app_);
  keybind_settings->SetShortcutControl(shortcuts_);
  root_context->setContextProperty("keybindSettingsModel", keybind_settings);
  connect(app_context.yandex_account, &AccountModel::dataLoaded, shortcuts_,
          [this, account = app_context.yandex_account] {
            shortcuts_->SetAccountId(account->GetAccountId());
          });
  connect(app_context.authorization_service, &IAuthorizationService::logout, shortcuts_, [this] {
    shortcuts_->SetEnabled(false);
    shortcuts_->SetAccountId({});
  });
  const auto rest_control = new RestControlService(
      app_context.settings, RestConfiguration(app_context.startup_options), app_context.app_);
  const auto rest_model = new RestViewModel(rest_control, app_context.app_);
  root_context->setContextProperty("restViewModel", rest_model);
  QObject::connect(app_context.authorization_service, &IAuthorizationService::authorized,
                   rest_model, [rest_model, settings = app_context.settings] {
                     if (settings->GetRestEnabled())
                       rest_model->SetEnabled(true);
                   });
  QObject::connect(app_context.authorization_service, &IAuthorizationService::logout, rest_model,
                   [rest_model] { rest_model->SetEnabled(false); });
  const auto themes = new Themes(app_context.app_);
  connect(app_context.settings, &Settings::accountPreferencesChanged, themes,
          [themes, app_context](const QString&) {
            themes->SetTheme(app_context.settings->GetCurrentTheme());
          });
  const auto update_window_mode = [app_context] {
    if (app_context.settings->GetTrayModeEnabled() ||
        (app_context.platform_service->NotchSupported() && app_context.settings->GetNotchEnabled()))
      app_context.platform_service->ShowOnlyInTray();
    else
      app_context.platform_service->ShowAsApp();
  };
  connect(app_context.settings, &Settings::trayModeEnabledChanged, app_context.app_, update_window_mode);
  connect(app_context.settings, &Settings::notchEnabledChanged, app_context.app_, update_window_mode);
  const auto router = new Router(app_context.app_);
  const auto scenarios_view_model =
      new ScenariosViewModel(app_context.scenario_service, app_context.app_);
  const auto home_view_model = new HomeViewModel(app_context.home_service, app_context.settings,
                                                 app_context.yandex_account, app_context.app_);
  const auto authorization_model =
      new AuthorizationModel(app_context.authorization_service, app_context.app_);
  const auto device_view_model = new DeviceViewModel(app_context.device_service, app_context.app_);
  const auto error_codes = new ErrorCodes(app_context.app_);
  const auto color_model = new ColorsModel(app_context.app_);
  const auto color_modes_model = new ColorModesModel(app_context.app_);
  const auto modes_model = new ModesModel(app_context.app_);
  const auto titles_list =
      new TitlesProvider(":/data/instances.json", "DataInstances", app_context.app_);
  const auto events_list =
      new TitlesProvider(":/data/propertiesEvents.json", "DataEvents", app_context.app_);
  const auto units_list = new UnitsList(app_context.app_);
  const auto device_data_model = device_view_model->GetDeviceData();
  const auto device_icons =
      new IconsProvider(":/data/deviceIcons.json", "devices", app_context.app_);
  const auto properties_icons =
      new IconsProvider(":/data/propertiesIcons.json", "properties", app_context.app_);
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
      &engine, &QQmlApplicationEngine::objectCreated, app_context_.app_,
      [themes, app_context, update_window_mode]() {
        update_window_mode();
        themes->SetTheme(app_context.settings->GetCurrentTheme());
      },
      Qt::QueuedConnection);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, app_context_.app_,
      [themes, app_context]() {
        qCritical() << "QQmlApplicationEngine::objectCreationFailed";
        QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);
}

int GuiApp::Start() {
  engine.loadFromModule("YandexHomeDesktop", "Main");

  const auto result = app_context_.app_->exec();
  // Let local IPC finish before destroying the controller. A bounded lease also
  // cleans up foreground workers if the GUI is killed before reaching here.
  QEventLoop shutdown;
  bool finished = false;
  connect(shortcuts_, &ShortcutControlService::shutdownFinished, &shutdown, [&] {
    finished = true;
    shutdown.quit();
  });
  QTimer::singleShot(4'000, &shutdown, &QEventLoop::quit);
  shortcuts_->Shutdown();
  if (!finished)
    shutdown.exec();
  return result;
}

void GuiApp::RegisterFonts() {
  const int id = QFontDatabase::addApplicationFont(":/fonts/Manrope-Regular.ttf");
  QFontDatabase::addApplicationFont(":/fonts/Manrope-Bold.ttf");

  const QString family = QFontDatabase::applicationFontFamilies(id).at(0);
  const QFont font(family);

  app_context_.app_->setFont(font);
}

void GuiApp::RegisterModels() {
  qmlRegisterType<CameraSnapshotModel>("YandexHomeDesktop.ViewModels", 1, 0, "CameraSnapshotModel");
  qmlRegisterUncreatableType<ShortcutViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
                                                "ShortcutViewModel", "Supplied by the application");
  qmlRegisterUncreatableType<SettingsTransferModel>(
      "YandexHomeDesktop.ViewModels", 1, 0, "SettingsTransferModel", "Supplied by the application");
  qmlRegisterUncreatableType<KeybindSettingsModel>(
      "YandexHomeDesktop.ViewModels", 1, 0, "KeybindSettingsModel", "Supplied by the application");
  qmlRegisterUncreatableType<RestViewModel>("YandexHomeDesktop.ViewModels", 1, 0, "RestViewModel",
                                            "Supplied by the application");
  qmlRegisterUncreatableType<DeviceViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
                                              "DeviceViewModel", "Supplied by the application");
  qmlRegisterUncreatableType<CapabilitiesModel>("YandexHomeDesktop.ViewModels", 1, 0,
                                                "CapabilitiesModel", "Supplied by the view model");
  qmlRegisterUncreatableType<PropertiesModel>("YandexHomeDesktop.ViewModels", 1, 0,
                                              "PropertiesModel", "Supplied by the view model");
  qmlRegisterUncreatableType<DeviceDataModel>("YandexHomeDesktop.ViewModels", 1, 0,
                                              "DeviceDataModel", "Supplied by the view model");
  qmlRegisterUncreatableType<HomeViewModel>("YandexHomeDesktop.ViewModels", 1, 0, "HomeViewModel",
                                            "Supplied by the application");
  qmlRegisterUncreatableType<DevicesModel>("YandexHomeDesktop.ViewModels", 1, 0, "DevicesModel",
                                           "Supplied by the application");
  qmlRegisterUncreatableType<RoomsModel>("YandexHomeDesktop.ViewModels", 1, 0, "RoomsModel",
                                         "Supplied by the application");
  qmlRegisterUncreatableType<HouseholdsModel>("YandexHomeDesktop.ViewModels", 1, 0,
                                              "HouseholdsModel", "Supplied by the application");
  qmlRegisterUncreatableType<ScenariosViewModel>(
      "YandexHomeDesktop.ViewModels", 1, 0, "ScenariosViewModel",
      "ScenariosViewModel is supplied by the application");
  qmlRegisterUncreatableType<ScenariosModel>("YandexHomeDesktop.ViewModels", 1, 0, "ScenariosModel",
                                             "ScenariosModel is supplied by ScenariosViewModel");
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
