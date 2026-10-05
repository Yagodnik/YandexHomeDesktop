#include "QmlTestEnvironment.h"

#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlContext>

#include "models/DevicesModel/DevicesFilterModel.h"
#include "models/RoomsModel/RoomsFilterModel.h"
#include "models/ScenariosModel/ScenariosViewModel.h"
#include "models/HomeViewModel.h"
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

  auto* api = new QmlTestHomeApi(engine);
  auto* home = new HomeService(api, engine);
  engine->rootContext()->setContextProperty("homeViewModel", new HomeViewModel(home, engine));
  auto* scenarios = new ScenarioService(api, engine);
  engine->rootContext()->setContextProperty("scenariosViewModel", new ScenariosViewModel(scenarios, engine));

  const QStringList names = {
    "capabilitiesModel", "propertiesModel", "deviceDataModel", "deviceController",
    "authorizationService", "yandexAccount", "settings", "platformService",
    "router", "errorCodes",
  };
  for (const auto& name : names) {
    engine->rootContext()->setContextProperty(name, new QmlTestModel(engine));
  }
}
