#include <QtTest>
#include "serialization/Serialization.h"

JSON_STRUCT(TestStruct,
  (int, a),
  (int, b),
  (int, c)
);

JSON_STRUCT(TestStruct2,
  (int, a),
  (int, b),
  (int, c),
  (TestStruct, test)
);

class TestMyClass final : public QObject {
  Q_OBJECT
private slots:
  static void FromJsonSimple() {
    QByteArray raw = R"({
      "a": 1,
      "b": 2,
      "c": 3
    })";

    QJsonDocument doc = QJsonDocument::fromJson(raw);

    auto t = Serialization::From<TestStruct>(doc.object());

    QCOMPARE(t.a, 1);
    QCOMPARE(t.b, 2);
    QCOMPARE(t.c, 3);
  }

  static void FromJsonSubObject() {
    QByteArray raw = R"({
      "a": 1,
      "b": 2,
      "c": 3,
      "test": {
        "a": 42,
        "b": 123,
        "c": 213
      }
    })";

    QJsonDocument doc = QJsonDocument::fromJson(raw);
    auto t = Serialization::From<TestStruct2>(doc.object());

    QCOMPARE(t.a, 1);
    QCOMPARE(t.b, 2);
    QCOMPARE(t.c, 3);
    QCOMPARE(t.test.a, 42);
    QCOMPARE(t.test.b, 123);
    QCOMPARE(t.test.c, 213);
  }

  static void ToJsonSimple() {
    TestStruct t1;
    t1.a = 42;
    t1.b = 123;
    t1.c = 213;

    const auto json = Serialization::To(t1);

    QVERIFY(json.value("a").isDouble());
    QVERIFY(!json.value("a").isUndefined());

    QVERIFY(json.value("b").isDouble());
    QVERIFY(json.value("c").isDouble());

    QCOMPARE(json.value("a").toInteger(), 42);
    QCOMPARE(json.value("b").toInteger(), 123);
    QCOMPARE(json.value("c").toInteger(), 213);
  }

  static void ToJsonSubObject() {
    TestStruct2 t1;
    t1.a = 1;
    t1.b = 2;
    t1.c = 3;
    t1.test.a = 42;
    t1.test.b = 123;
    t1.test.c = 213;

    const auto json = Serialization::To(t1);

    QCOMPARE(json.value("a").toInteger(), 1);
    QCOMPARE(json.value("b").toInteger(), 2);
    QCOMPARE(json.value("c").toInteger(), 3);
    QVERIFY(json.value("test").isObject());

    auto object = json.value("test").toObject();
    QCOMPARE(object.value("a").toInteger(), 42);
    QCOMPARE(object.value("b").toInteger(), 123);
    QCOMPARE(object.value("c").toInteger(), 213);
  }
};

QTEST_MAIN(TestMyClass)
#include "SerializationTest.moc"
