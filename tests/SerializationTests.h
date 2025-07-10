#pragma once

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
  static void FromJsonSimple();
  static void FromJsonSubObject();
  static void ToJsonSimple();
  static void ToJsonSubObject();
  static void JsonEnumTest();
  static void JsonEnumSerializationTest();
  static void JsonEnumDeserializationTest();
  static void JsonEnumDeserialization2Test();
  static void JsonIntArrayTest();
  static void JsonArrayTest();
  static void JsonArraySerializationTest();
  static void JsonArraySerializationTest2();
  static void TripTest();
  static void JsonEnumNoValueTest();
};

