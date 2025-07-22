#include "SerializationTests.h"

void SerializationTests::FromJsonSimple() {
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

void SerializationTests::FromJsonSubObject() {
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

void SerializationTests::ToJsonSimple() {
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

void SerializationTests::ToJsonSubObject() {
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

void SerializationTests::JsonEnumTest() {
  MyEnumeration my_enum;

  QVERIFY(my_enum["MyEnum::A"] == MyEnumeration::A);
  QVERIFY(my_enum["MyEnum::A"] != MyEnumeration::B);
  QVERIFY(my_enum["MagicString!"] == MyEnumeration::Magic);

  QVERIFY(my_enum[MyEnumeration::A] == "MyEnum::A");
  QVERIFY(my_enum[MyEnumeration::B] != "MyEnum::A");
  QVERIFY(my_enum[MyEnumeration::Magic] == "MagicString!");
}

void SerializationTests::JsonEnumSerializationTest() {
  QByteArray raw = R"({
    "value": 213,
    "enum_value": "MagicString!"
  })";

  QJsonDocument doc = QJsonDocument::fromJson(raw);

  auto t = Serialization::From<StructWithEnum>(doc.object());

  QVERIFY(t.value == 213);
  QVERIFY(t.enum_value == MyEnumeration::Magic);
}

void SerializationTests::JsonEnumDeserializationTest() {
  const StructWithEnum s {
    .value = 213,
    .enum_value = MyEnumeration::Magic
  };

  auto j = Serialization::To<StructWithEnum>(s);

  QVERIFY(j.value("value").toInteger() == 213);
  QVERIFY(j.value("enum_value").toString() == "MagicString!");
}

void SerializationTests::JsonEnumDeserialization2Test() {
  const StructWithDoubleEnum s {
    .enum_value1 = MyEnumeration::Magic,
    .enum_value2 = MyEnumeration::B
  };

  auto j = Serialization::To<StructWithDoubleEnum>(s);

  QVERIFY(j.value("enum_value1").toString() == "MagicString!");
  QVERIFY(j.value("enum_value2").toString() == "MyEnum::B");
}

void SerializationTests::JsonIntArrayTest() {
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

void SerializationTests::JsonArrayTest() {
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

void SerializationTests::JsonArraySerializationTest() {
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

void SerializationTests::JsonArraySerializationTest2() {
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

void SerializationTests::TripTest() {
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

void SerializationTests::JsonEnumNoValueTest() {
  const QString value = "something which you definitely wont see here)";
  const auto enum_value = TransportEnum::AsValue(value);
}
