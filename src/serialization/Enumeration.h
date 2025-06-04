#pragma once

#define JSON_ENUM_ITEM_NAME(str, val) val
#define JSON_ENUM_ITEM_PAIR(str, val) std::make_pair(str, val)

#define JSON_ENUM_VALUES_1(p1) JSON_ENUM_ITEM_NAME p1
#define JSON_ENUM_VALUES_2(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_1(__VA_ARGS__)
#define JSON_ENUM_VALUES_3(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_2(__VA_ARGS__)
#define JSON_ENUM_VALUES_4(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_3(__VA_ARGS__)
#define JSON_ENUM_VALUES_5(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_4(__VA_ARGS__)
#define JSON_ENUM_VALUES_6(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_5(__VA_ARGS__)
#define JSON_ENUM_VALUES_7(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_6(__VA_ARGS__)
#define JSON_ENUM_VALUES_8(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_7(__VA_ARGS__)
#define JSON_ENUM_VALUES_9(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_8(__VA_ARGS__)
#define JSON_ENUM_VALUES_10(p1, ...) JSON_ENUM_ITEM_NAME p1, JSON_ENUM_VALUES_9(__VA_ARGS__)

#define JSON_ENUM_PAIRS_1(p1) JSON_ENUM_ITEM_PAIR p1
#define JSON_ENUM_PAIRS_2(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_1(__VA_ARGS__)
#define JSON_ENUM_PAIRS_3(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_2(__VA_ARGS__)
#define JSON_ENUM_PAIRS_4(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_3(__VA_ARGS__)
#define JSON_ENUM_PAIRS_5(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_4(__VA_ARGS__)
#define JSON_ENUM_PAIRS_6(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_5(__VA_ARGS__)
#define JSON_ENUM_PAIRS_7(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_6(__VA_ARGS__)
#define JSON_ENUM_PAIRS_8(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_7(__VA_ARGS__)
#define JSON_ENUM_PAIRS_9(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_8(__VA_ARGS__)
#define JSON_ENUM_PAIRS_10(p1, ...) JSON_ENUM_ITEM_PAIR p1, JSON_ENUM_PAIRS_9(__VA_ARGS__)

#define JSON_ENUM_SELECT(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME

#define JSON_ENUM_VALUES(...) JSON_ENUM_SELECT(__VA_ARGS__, \
  JSON_ENUM_VALUES_10,  \
  JSON_ENUM_VALUES_9,   \
  JSON_ENUM_VALUES_8,   \
  JSON_ENUM_VALUES_7,   \
  JSON_ENUM_VALUES_6,   \
  JSON_ENUM_VALUES_5,   \
  JSON_ENUM_VALUES_4,   \
  JSON_ENUM_VALUES_3,   \
  JSON_ENUM_VALUES_2,   \
  JSON_ENUM_VALUES_1    \
)(__VA_ARGS__)

#define JSON_ENUM_PAIRS(...) JSON_ENUM_SELECT(__VA_ARGS__, \
  JSON_ENUM_PAIRS_10,   \
  JSON_ENUM_PAIRS_9,    \
  JSON_ENUM_PAIRS_8,    \
  JSON_ENUM_PAIRS_7,    \
  JSON_ENUM_PAIRS_6,    \
  JSON_ENUM_PAIRS_5,    \
  JSON_ENUM_PAIRS_4,    \
  JSON_ENUM_PAIRS_3,    \
  JSON_ENUM_PAIRS_2,    \
  JSON_ENUM_PAIRS_1     \
)(__VA_ARGS__)

#define JSON_ENUMERATION(name, ...)                                 \
struct name {                                                       \
enum Type {                                                         \
JSON_ENUM_VALUES(__VA_ARGS__)                                       \
};                                                                  \
                                                                    \
static const QMap<QString, Type> mapping;                           \
                                                                    \
static Type operator[](const QString& key) {                        \
  auto it = mapping.find(key);                                      \
  if (it == mapping.end()) {                                        \
    throw std::out_of_range("Invalid enum string key: " +           \
        key.toStdString());                                         \
  }                                                                 \
  return *it;                                                       \
}                                                                   \
};                                                                  \
                                                                    \
const QMap<QString, name::Type> name::mapping = {                   \
JSON_ENUM_PAIRS(__VA_ARGS__)                                        \
};                                                                  \
