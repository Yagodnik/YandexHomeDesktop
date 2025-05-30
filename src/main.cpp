#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "api/YandexHomeApi.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    YandexHomeApi api;

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("YandexHomeDesktop", "Main");

    return app.exec();
}
