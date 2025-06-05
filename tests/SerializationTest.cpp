#include <QtTest>
#include "serialization/Serialization.h"

JSON_STRUCT(TestStruct,
  (int, a),
  (double, b),
  (int, c)
);

JSON_STRUCT(TestStruct2,
  (int, a),
  (int, b),
  (int, c),
  (TestStruct, test)
);

JSON_ENUMERATION(MyEnumeration,
  ("MyEnum::A", A),
  ("MyEnum::B", B),
  ("MyEnum::C", C),
  ("MagicString!", Magic)
);

JSON_STRUCT(StructWithEnum,
  (int, value),
  (MyEnumeration, enum_value)
);

JSON_STRUCT(StructWithDoubleEnum,
  (MyEnumeration, enum_value1),
  (MyEnumeration, enum_value2)
);

class TestMyClass final : public QObject {
  Q_OBJECT
private slots:
  static void FromJsonSimple() {
    QByteArray raw = R"({
      "a": 1,
      "b": 2.71,
      "c": 3
    })";

    QJsonDocument doc = QJsonDocument::fromJson(raw);

    auto t = Serialization::From<TestStruct>(doc.object());

    QCOMPARE(t.a, 1);
    QCOMPARE(t.b, 2.71);
    QCOMPARE(t.c, 3);
  }

  static void FromJsonSubObject() {
    QByteArray raw = R"({
      "a": 1,
      "b": 2,
      "c": 3,
      "d" : 4,
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

  static void JsonEnumTest() {
    MyEnumeration my_enum;

    QVERIFY(my_enum["MyEnum::A"] == MyEnumeration::A);
    QVERIFY(my_enum["MyEnum::A"] != MyEnumeration::B);
    QVERIFY(my_enum["MagicString!"] == MyEnumeration::Magic);

    QVERIFY(my_enum[MyEnumeration::A] == "MyEnum::A");
    QVERIFY(my_enum[MyEnumeration::B] != "MyEnum::A");
    QVERIFY(my_enum[MyEnumeration::Magic] == "MagicString!");
  }

  static void JsonEnumSerializationTest() {
    QByteArray raw = R"({
      "value": 213,
      "enum_value": "MagicString!"
    })";

    QJsonDocument doc = QJsonDocument::fromJson(raw);

    auto t = Serialization::From<StructWithEnum>(doc.object());

    QVERIFY(t.value == 213);
    QVERIFY(t.enum_value == MyEnumeration::Magic);
  }

  static void JsonEnumDeserializationTest() {
    const StructWithEnum s {
      .value = 213,
      .enum_value = MyEnumeration::Magic
    };

    auto j = Serialization::To<StructWithEnum>(s);

    QVERIFY(j.value("value").toInteger() == 213);
    QVERIFY(j.value("enum_value").toString() == "MagicString!");
  }

  static void JsonEnumDeserialization2Test() {
    const StructWithDoubleEnum s {
      .enum_value1 = MyEnumeration::Magic,
      .enum_value2 = MyEnumeration::B
    };

    auto j = Serialization::To<StructWithDoubleEnum>(s);

    QVERIFY(j.value("enum_value1").toString() == "MagicString!");
    QVERIFY(j.value("enum_value2").toString() == "MyEnum::B");
  }
};

QTEST_MAIN(TestMyClass)
#include "SerializationTest.moc"
