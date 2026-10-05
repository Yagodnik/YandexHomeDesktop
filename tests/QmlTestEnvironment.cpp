#include "QmlTestEnvironment.h"

#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlContext>

#include "models/DevicesModel/DevicesFilterModel.h"
#include "models/RoomsModel/RoomsFilterModel.h"
#include "utils/Themes.h"

void initializeQmlTestEnvironment(QQmlEngine* engine) {
  static const bool registered = [] {
    qmlRegisterType<DevicesFilterModel>("YandexHomeDesktop.Models", 1, 0, "DevicesFilterModel");
    qmlRegisterType<RoomsFilterModel>("YandexHomeDesktop.Models", 1, 0, "RoomsFilterModel");
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

  const QStringList names = {
    "devicesModel", "roomsModel", "householdsModel", "scenariosModel",
    "capabilitiesModel", "propertiesModel", "deviceDataModel", "deviceController",
    "authorizationService", "yandexAccount", "settings", "platformService",
    "router", "errorCodes",
  };
  for (const auto& name : names) {
    engine->rootContext()->setContextProperty(name, new QmlTestModel(engine));
  }
}
