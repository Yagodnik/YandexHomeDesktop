#include <iostream>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QJsonValue>
#include <QJsonObject>
#include "api/YandexHomeApi.h"
#include "serialization/Serialization.h"

JSON_ENUMERATION(MyEnumeration,
  ("MyEnum::A", A),
  ("MyEnum::B", B),
  ("MyEnum::C", C),
  ("MagicString!", Magic)
);

JSON_STRUCT(SubStruct,
  (QString, str_value),
  (int, a),
  (int, b),
  (int, c)
);

JSON_STRUCT(Person,
  (QString, name),
  (int, age),
  (float, f_value),
  (SubStruct, sub_struct)
);

int main(int argc, char *argv[]) {
  QByteArray raw = R"({
    "name":"Abebebeb",
    "age":30,
    "f_value": 3.14,
    "sub_struct": {
      "str_value": "Hello World!",
      "a": 1,
      "b": 2,
      "c": 3
    }
  })";

  QJsonDocument doc = QJsonDocument::fromJson(raw);
  QJsonObject j = doc.object();

  auto p = Serialization::From<Person>(j);

  qDebug() << p.name << ", " << p.age  << ", " << p.f_value;
  qDebug() << p.sub_struct.str_value;

  QJsonObject j2 = Serialization::To(p);
  QJsonDocument doc2(j2);
  std::cout << doc2.toJson(QJsonDocument::Indented).toStdString() << std::endl;

  MyEnumeration my_enum;
  std::cout << (my_enum["MyEnum::A"] == MyEnumeration::A) << std::endl;
  std::cout << (my_enum["MyEnum::A"] == MyEnumeration::B) << std::endl;
  std::cout << (my_enum["MagicString!"] == MyEnumeration::Magic) << std::endl;

  return 0;

    // QGuiApplication app(argc, argv);
    //
    // QQmlApplicationEngine engine;
    // QObject::connect(
    //     &engine,
    //     &QQmlApplicationEngine::objectCreationFailed,
    //     &app,
    //     []() { QCoreApplication::exit(-1); },
    //     Qt::QueuedConnection);
    // engine.loadFromModule("YandexHomeDesktop", "Main");
    //
    // return app.exec();
}

