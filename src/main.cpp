#include <iostream>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "api/YandexHomeApi.h"
#include "api/model/UserInfo.h"
#include "auth/YandexAuth.h"

int main(int argc, char *argv[]) {
  // QFile test_file("../../../../resources/UserInfoTest.txt");
  // if (!test_file.open(QIODevice::ReadOnly)) {
  //   qDebug() << "Failed to open file";
  //   return 1;
  // }
  //
  // QJsonDocument testDoc = QJsonDocument::fromJson(test_file.readAll());
  // QJsonObject response_json = testDoc.object();
  //
  // auto response = Serialization::From<UserInfo>(response_json);
  //
  // qDebug() << "Status: " << (response.status == Status::Ok ? "Ok" : "Error");
  //
  // qDebug() << "Request ID:" << response.request_id;
  //
  // for (const auto& room : response.rooms) {
  //   qDebug() << "Room ID:" << room.id << "Name:" << room.name;
  // }
  // qDebug() << "";
  //
  // for (const auto& group : response.groups) {
  //   qDebug() << "Group ID:" << group.id << "Name:" << group.name;
  // }
  // qDebug() << "";
  //
  // for (const auto& device : response.devices) {
  //   qDebug() << "Device ID:" << device.id << "Name:" << "WOP";
  // }
  // qDebug() << "";
  //
  // for (const auto& scenario : response.scenarios) {
  //   qDebug() << "Scenario ID:" << scenario.id << "Name:" << scenario.name;
  // }
  // qDebug() << "";
  //
  // for (const auto& household : response.households) {
  //   qDebug() << "Households ID:" << household.id << "Name:" << household.name;
  // }
  //
  // return 0;

  QGuiApplication app(argc, argv);

  qmlRegisterType<YandexAuth>("Yandex", 1, 0, "YandexAuth");

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

