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

JSON_STRUCT(ArrayItem,
  (int, id),
  (QString, name)
);

JSON_STRUCT(StructArray,
  (double, d_value),
  (QList<ArrayItem>, test_array)
);

JSON_STRUCT(IntArrayStruct,
  (double, d_value),
  (QList<int>, test_array)
);

/* --------- */

JSON_ENUMERATION(TransportEnum,
  ("Car", Car),
  ("Plane", Plane),
  ("Ship", Ship)
);

JSON_STRUCT(Trip,
  (QString, trip_name),
  (int, timestamp),
  (TransportEnum, transport_type),
  (QList<QString>, points)
);

JSON_STRUCT(TripsList,
  (QString, date),
  (int, id),
  (QList<Trip>, trips)
);

class SerializationTests final : public QObject {
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

  static void JsonIntArrayTest() {
    QByteArray raw = R"({
      "d_value": 2.71,
      "test_array": [
        213, 123, 4321, 1111
      ]
    })";

    QJsonDocument doc = QJsonDocument::fromJson(raw);
    auto t = Serialization::From<IntArrayStruct>(doc.object());

    QCOMPARE(t.d_value, 2.71);
    QCOMPARE(t.test_array.size(), 4);

    QCOMPARE(t.test_array[0], 213);
    QCOMPARE(t.test_array[1], 123);
    QCOMPARE(t.test_array[2], 4321);
    QCOMPARE(t.test_array[3], 1111);
  }

  static void JsonArrayTest() {
    QByteArray raw = R"({
      "d_value": 2.71,
      "test_array": [
        {
          "id": 213,
          "name": "abebe"
        },
        {
          "id": 123,
          "name": "yourname"
        }
      ]
    })";

    QJsonDocument doc = QJsonDocument::fromJson(raw);
    auto t = Serialization::From<StructArray>(doc.object());

    QCOMPARE(t.d_value, 2.71);
    QCOMPARE(t.test_array.size(), 2);

    QCOMPARE(t.test_array[0].id, 213);
    QCOMPARE(t.test_array[0].name, "abebe");

    QCOMPARE(t.test_array[1].id, 123);
    QCOMPARE(t.test_array[1].name, "yourname");
  }

  static void JsonArraySerializationTest() {
    IntArrayStruct s = {
      .d_value = 2.2,
      .test_array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
    };

    auto j = Serialization::To<IntArrayStruct>(s);

    QVERIFY(j.value("d_value").toDouble() == 2.2);
    QVERIFY(j.value("test_array").isArray());

    auto arr = j.value("test_array").toArray();

    QCOMPARE(arr.size(), 10);
    QCOMPARE(arr[0], 1);
    QCOMPARE(arr[1], 2);
    QCOMPARE(arr[2], 3);
    QCOMPARE(arr[3], 4);
    QCOMPARE(arr[4], 5);
    QCOMPARE(arr[5], 6);
    QCOMPARE(arr[6], 7);
    QCOMPARE(arr[7], 8);
    QCOMPARE(arr[8], 9);
    QCOMPARE(arr[9], 10);
  }

  static void JsonArraySerializationTest2() {
    StructArray s = {
      .d_value = 2.2,
      .test_array = {
        {
          .id = 213,
          .name = "abe",
        },
        {
          .id = 123,
          .name = "abc",
        }
      }
    };

    auto j = Serialization::To<StructArray>(s);

    QCOMPARE(j.value("d_value").toDouble(), 2.2);
    QVERIFY(j.value("test_array").isArray());

    auto arr = j.value("test_array").toArray();
    QCOMPARE(arr.size(), 2);
    QCOMPARE(arr[0].toObject().value("id").toInteger(), 213);
    QCOMPARE(arr[0].toObject().value("name").toString(), "abe");

    QCOMPARE(arr[1].toObject().value("id").toInteger(), 123);
    QCOMPARE(arr[1].toObject().value("name").toString(), "abc");
  }

  static void TripTest() {
    QByteArray raw = R"({
        "date": "2025-06-06",
        "id": 1234,
        "trips": [
          {
            "trip_name": "First Trip",
            "timestamp": 123456789,
            "transport_type": "Plane",
            "points": [ "Saint-Petersburg", "Moscow", "Rostov" ]
          },
          {
            "trip_name": "Second Trip",
            "timestamp": 987654321,
            "transport_type": "Car",
            "points": [ "Saint-Petersburg", "Perm", "Yakutsk" ]
          }
        ]
    })";

    QJsonDocument doc = QJsonDocument::fromJson(raw);

    const auto data = Serialization::From<TripsList>(doc.object());

    QCOMPARE(data.date, "2025-06-06");
    QCOMPARE(data.id, 1234);
    QCOMPARE(data.trips.size(), 2);

    auto trip1 = data.trips.at(0);
    auto trip2 = data.trips.at(1);

    QCOMPARE(trip1.trip_name, "First Trip");
    QCOMPARE(trip2.trip_name, "Second Trip");
    QCOMPARE(trip1.timestamp, 123456789);
    QCOMPARE(trip2.timestamp, 987654321);


    auto points1 = trip1.points;
    auto points2 = trip2.points;

    QCOMPARE(points1.size(), 3);
    QCOMPARE(points2.size(), 3);

    QCOMPARE(points1[0], "Saint-Petersburg");
    QCOMPARE(points1[1], "Moscow");
    QCOMPARE(points1[2], "Rostov");

    QCOMPARE(points2[0], "Saint-Petersburg");
    QCOMPARE(points2[1], "Perm");
    QCOMPARE(points2[2], "Yakutsk");

    QCOMPARE(trip1.transport_type, TransportEnum::Plane);
    QCOMPARE(trip2.transport_type, TransportEnum::Car);
  }
};

QTEST_MAIN(SerializationTests)

#include "SerializationTest.moc"
