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
#include "models/DeviceModel/DeviceViewModel.h"
#include "iot/capabilities/OnOffCapability.h"
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
    qmlRegisterType<OnOffCapability>("YandexHomeDesktop.Capabilities", 1, 0, "OnOff");
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

  const QStringList names = {
    "authorizationService", "yandexAccount", "settings", "platformService",
    "router", "errorCodes",
  };
  for (const auto& name : names) {
    engine->rootContext()->setContextProperty(name, new QmlTestModel(engine));
  }
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
