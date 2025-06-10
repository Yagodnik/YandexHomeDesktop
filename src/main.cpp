#include <iostream>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "api/YandexHomeApi.h"
#include "api/model/UserInfo.h"
#include "auth/AuthorizationService.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  const auto root_context = engine.rootContext();

  root_context->setContextProperty("authorizationService", new AuthorizationService());

  QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreationFailed,
    &app,
    []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);
  engine.loadFromModule("YandexHomeDesktop", "Main");

  return app.exec();
}

