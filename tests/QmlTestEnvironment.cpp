#include "QmlTestEnvironment.h"

#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlContext>
#include <QJsonDocument>
#include <QUuid>

#include "models/DevicesModel/DevicesFilterModel.h"
#include "models/RoomsModel/RoomsFilterModel.h"
#include "models/ScenariosModel/ScenariosViewModel.h"
#include "models/HomeViewModel.h"
#include "models/KeybindSettingsModel.h"
#include "models/SettingsTransferModel.h"
#include <QTemporaryDir>
#include "models/DeviceModel/DeviceViewModel.h"
#include "iot/capabilities/OnOffCapability.h"
#include "iot/capabilities/RangeCapability.h"
#include "iot/capabilities/ColorSettingCapability.h"
#include "iot/capabilities/ModesCapability.h"
#include "iot/capabilities/ToggleCapability.h"
#include "iot/properties/FloatProperty.h"
#include "iot/properties/EventProperty.h"
#include "serialization/Serialization.h"
#include "models/ColorsModel/ColorsFilterModel.h"
#include "models/ColorsModel/ColorsModel.h"
#include "models/ColorsModel/ColorModesFilterModel.h"
#include "models/ColorsModel/ColorModesModel.h"
#include "models/ModesModel/ModesFilterModel.h"
#include "models/ModesModel/ModesModel.h"
#include "utils/UnitsList.h"
#include "utils/Themes.h"
#include "utils/IconsProvider.h"

namespace {
class QmlTestHomeApi final : public QObject, public IHomeApi {
public:
  using QObject::QObject;
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>> handler) override {
    handler(QList<ScenarioObject>{});
  }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void> handler) override {
    handler(ApiResult<void>{});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo> handler) override { handler(UserInfo{}); }
  void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo>) override { qFatal("Unexpected device request"); }
  void PerformActions(const QList<DeviceActionsObject>&, QObject*, ApiResultHandler<void>) override {
    qFatal("Unexpected action request");
  }
};
}

void initializeQmlTestEnvironment(QQmlEngine* engine) {
  static const bool registered = [] {
    qmlRegisterUncreatableType<SettingsTransferModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "SettingsTransferModel", "Supplied by the application");
    qmlRegisterUncreatableType<KeybindSettingsModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "KeybindSettingsModel", "Supplied by the application");
    qmlRegisterType<OnOffCapability>("YandexHomeDesktop.Capabilities", 1, 0, "OnOff");
    qmlRegisterType<RangeCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Range");
    qmlRegisterType<ColorSettingCapability>("YandexHomeDesktop.Capabilities", 1, 0, "ColorSetting");
    qmlRegisterType<ModesCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Modes");
    qmlRegisterType<ToggleCapability>("YandexHomeDesktop.Capabilities", 1, 0, "Toggle");
    qmlRegisterType<FloatProperty>("YandexHomeDesktop.Properties", 1, 0, "Float");
    qmlRegisterType<EventProperty>("YandexHomeDesktop.Properties", 1, 0, "Event");
    qmlRegisterType<ColorsFilterModel>("YandexHomeDesktop.Models", 1, 0, "ColorsFilterModel");
    qmlRegisterType<ColorModesFilterModel>("YandexHomeDesktop.Models", 1, 0, "ColorModesFilterModel");
    qmlRegisterType<ModesFilterModel>("YandexHomeDesktop.Models", 1, 0, "ModesFilterModel");
    qmlRegisterUncreatableType<RestViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "RestViewModel", "Supplied by the application");
    qmlRegisterType<DevicesFilterModel>("YandexHomeDesktop.Models", 1, 0, "DevicesFilterModel");
    qmlRegisterType<RoomsFilterModel>("YandexHomeDesktop.Models", 1, 0, "RoomsFilterModel");
    qmlRegisterUncreatableType<HomeViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "HomeViewModel", "Supplied by the application");
    qmlRegisterUncreatableType<DevicesModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "DevicesModel", "Supplied by the application");
    qmlRegisterUncreatableType<RoomsModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "RoomsModel", "Supplied by the application");
    qmlRegisterUncreatableType<HouseholdsModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "HouseholdsModel", "Supplied by the application");
    qmlRegisterUncreatableType<ScenariosViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "ScenariosViewModel", "Supplied by the application");
    qmlRegisterUncreatableType<ScenariosModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "ScenariosModel", "Supplied by the view model");
    qmlRegisterUncreatableType<DeviceViewModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "DeviceViewModel", "Supplied by the application");
    qmlRegisterUncreatableType<CapabilitiesModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "CapabilitiesModel", "Supplied by the view model");
    qmlRegisterUncreatableType<PropertiesModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "PropertiesModel", "Supplied by the view model");
    qmlRegisterUncreatableType<DeviceDataModel>("YandexHomeDesktop.ViewModels", 1, 0,
      "DeviceDataModel", "Supplied by the view model");
    return true;
  }();
  Q_UNUSED(registered);

  const auto fontId = QFontDatabase::addApplicationFont(":/fonts/Manrope-Regular.ttf");
  const auto families = QFontDatabase::applicationFontFamilies(fontId);
  if (families.isEmpty()) {
    qFatal("The bundled test font could not be loaded");
  }
  QGuiApplication::setFont(QFont(families.constFirst()));

  auto* themes = new Themes(engine);
  themes->SetTheme(0);
  engine->rootContext()->setContextProperty("themes", themes);
  engine->rootContext()->setContextProperty("deviceIcons", new IconsProvider(":/data/deviceIcons.json", "devices", engine));
  engine->rootContext()->setContextProperty("iotTitles", new TitlesProvider(":/data/instances.json", "DataInstances", engine));
  engine->rootContext()->setContextProperty("unitsList", new UnitsList(engine));
  engine->rootContext()->setContextProperty("colorModel", new ColorsModel(engine));
  engine->rootContext()->setContextProperty("colorModesModel", new ColorModesModel(engine));
  engine->rootContext()->setContextProperty("modesModel", new ModesModel(engine));
  engine->rootContext()->setContextProperty("propertiesIcons", new IconsProvider(":/data/propertiesIcons.json", "properties", engine));
  engine->rootContext()->setContextProperty("eventTitles", new TitlesProvider(":/data/propertiesEvents.json", "DataEvents", engine));

  auto* api = new QmlTestHomeApi(engine);
  auto* home = new HomeService(api, engine);
  engine->rootContext()->setContextProperty("homeViewModel", new HomeViewModel(home, engine));
  auto* scenarios = new ScenarioService(api, engine);
  engine->rootContext()->setContextProperty("scenariosViewModel", new ScenariosViewModel(scenarios, engine));

  auto* device_api = new QmlTestDeviceApi(engine);
  auto* device_service = new DeviceService(device_api, engine);
  auto* device = new DeviceViewModel(device_service, engine);
  engine->rootContext()->setContextProperty("deviceTestApi", device_api);
  engine->rootContext()->setContextProperty("deviceViewModel", device);
  engine->rootContext()->setContextProperty("deviceController", device);
  engine->rootContext()->setContextProperty("deviceDataModel", device->GetDeviceData());
  engine->rootContext()->setContextProperty("capabilitiesModel", device->GetCapabilities());
  engine->rootContext()->setContextProperty("propertiesModel", device->GetProperties());

  auto* rest_server = new QmlTestRestServer(engine);
  auto* rest_settings = new Settings(engine, true);
  auto* rest_control = new RestControlService(rest_settings,
    {rest_server->ControlName(), {}, {}, true}, engine);
  engine->rootContext()->setContextProperty("restTestServer", rest_server);
  engine->rootContext()->setContextProperty("restViewModel", new RestViewModel(rest_control, engine));
  engine->rootContext()->setContextProperty("settings", rest_settings);
  QObject::connect(rest_settings, &Settings::accountPreferencesChanged, themes,
    [themes, rest_settings](const QString&) { themes->SetTheme(rest_settings->GetCurrentTheme()); });
  auto* keybind_settings = new KeybindSettingsModel(rest_settings, nullptr, engine);
  keybind_settings->SetAccountId("test-account");
  engine->rootContext()->setContextProperty("keybindSettingsModel", keybind_settings);
  auto* transfer = new SettingsTransferModel(new SettingsTransferService(rest_settings, engine), nullptr, engine);
  transfer->SetAccountId("test-account");
  engine->rootContext()->setContextProperty("settingsTransferModel", transfer);
  auto* transfer_directory = new QTemporaryDir;
  QObject::connect(engine, &QObject::destroyed, [transfer_directory] { delete transfer_directory; });
  engine->rootContext()->setContextProperty("settingsTransferTestFile", QUrl::fromLocalFile(transfer_directory->filePath("settings.json")));
  QObject::connect(device_api, &QmlTestDeviceApi::fixtureReset, transfer, [transfer] {
    transfer->SetAccountId({});
    transfer->SetAccountId("test-account");
  });
  QObject::connect(device_api, &QmlTestDeviceApi::testAccountSelected, transfer, &SettingsTransferModel::SetAccountId);
  QObject::connect(device_api, &QmlTestDeviceApi::fixtureReset, rest_settings, [rest_settings, keybind_settings] {
    rest_settings->Reset();
    keybind_settings->SetAccountId("test-account");
  });
  QObject::connect(device_api, &QmlTestDeviceApi::testAccountSelected, keybind_settings, &KeybindSettingsModel::SetAccountId);

  const QStringList names = {
    "authorizationService", "yandexAccount", "platformService",
    "router", "errorCodes",
  };
  for (const auto& name : names) {
    engine->rootContext()->setContextProperty(name, new QmlTestModel(engine));
  }
}

void QmlTestDeviceApi::ReplyFromFixture(const QVariantMap& device) {
  if (!context_ || !handler_ || device.value("id").toString() != device_id_) {
    qFatal("No matching pending demo device request");
  }
  auto handler = std::move(handler_);
  handler(Serialization::From<DeviceInfo>(QJsonObject::fromVariantMap(device)));
}

QmlTestRestServer::QmlTestRestServer(QObject* parent) : QObject(parent) {
  if (!server_.listen("yh-rest-test-" + QUuid::createUuid().toString(QUuid::Id128)))
    qFatal("Cannot start local test control server");
  connect(&server_, &QLocalServer::newConnection, this, [this] {
    while (auto* socket = server_.nextPendingConnection()) {
      auto buffer = std::make_shared<QByteArray>();
      connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
      connect(socket, &QLocalSocket::readyRead, socket, [this, socket, buffer] {
        *buffer += socket->readAll();
        if (!buffer->contains('\n')) return;
        const auto command = QJsonDocument::fromJson(*buffer).object()["command"].toString();
        if (command == "disable") { running_ = false; ++disable_count_; emit changed(); }
        const QJsonObject result{{"ok", true}, {"enabled", running_}, {"running", running_},
          {"port", 8766}, {"url", "http://127.0.0.1:8766/v1"}, {"fixture", true}};
        socket->write(QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n');
        socket->disconnectFromServer();
      });
    }
  });
}
