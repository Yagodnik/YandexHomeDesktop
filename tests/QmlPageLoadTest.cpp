#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>
#include <QDebug>
#include <memory>
#include "QmlTestEnvironment.h"

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  if (argc != 3) {
    qCritical() << "Expected a QML page and build import directory";
    return 1;
  }

  QQmlEngine engine;
  initializeQmlTestEnvironment(&engine);
  engine.addImportPath(QString::fromLocal8Bit(argv[2]));
  bool warningsOccurred = false;
  QObject::connect(&engine, &QQmlEngine::warnings, &engine,
    [&warningsOccurred](const QList<QQmlError>&) { warningsOccurred = true; });
  QQmlComponent page(&engine, QUrl::fromLocalFile(QString::fromLocal8Bit(argv[1])),
                     QQmlComponent::PreferSynchronous);
  if (page.status() != QQmlComponent::Ready) {
    qCritical().noquote() << page.errorString();
    return 1;
  }
  QVariantMap initial_properties;
  if (QString::fromLocal8Bit(argv[1]).endsWith("SettingsPage.qml")) {
    initial_properties.insert("restModel", engine.rootContext()->contextProperty("restViewModel"));
  }
  if (QString::fromLocal8Bit(argv[1]).endsWith("ScenariosPage.qml")) {
    initial_properties.insert("viewModel", engine.rootContext()->contextProperty("scenariosViewModel"));
  }
  if (QString::fromLocal8Bit(argv[1]).endsWith("DevicesPage.qml")) {
    initial_properties.insert("viewModel", engine.rootContext()->contextProperty("homeViewModel"));
  }
  std::unique_ptr<QObject> instance(page.createWithInitialProperties(initial_properties));
  if (!instance) {
    qCritical().noquote() << page.errorString();
    return 1;
  }
  instance->setProperty("width", 350);
  instance->setProperty("height", 460);
  QCoreApplication::processEvents();
  return warningsOccurred ? 1 : 0;
}
