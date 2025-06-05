#pragma once

#define JSON_ENUM_ITEM_NAME(str, val) val
#define JSON_ENUM_ITEM_PAIR(str, val) std::make_pair(str, val)
#define JSON_ENUM_ITEM_BI_PAIR(str, val) std::make_pair(val, str)

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

#define JSON_ENUM_BI_PAIRS_1(p1) JSON_ENUM_ITEM_BI_PAIR p1
#define JSON_ENUM_BI_PAIRS_2(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_1(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_3(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_2(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_4(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_3(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_5(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_4(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_6(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_5(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_7(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_6(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_8(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_7(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_9(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_8(__VA_ARGS__)
#define JSON_ENUM_BI_PAIRS_10(p1, ...) JSON_ENUM_ITEM_BI_PAIR p1, JSON_ENUM_BI_PAIRS_9(__VA_ARGS__)

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

#define JSON_ENUM_BI_PAIRS(...) JSON_ENUM_SELECT(__VA_ARGS__, \
  JSON_ENUM_BI_PAIRS_10,   \
  JSON_ENUM_BI_PAIRS_9,    \
  JSON_ENUM_BI_PAIRS_8,    \
  JSON_ENUM_BI_PAIRS_7,    \
  JSON_ENUM_BI_PAIRS_6,    \
  JSON_ENUM_BI_PAIRS_5,    \
  JSON_ENUM_BI_PAIRS_4,    \
  JSON_ENUM_BI_PAIRS_3,    \
  JSON_ENUM_BI_PAIRS_2,    \
  JSON_ENUM_BI_PAIRS_1     \
)(__VA_ARGS__)


struct IEnumeration {};

#define JSON_ENUMERATION(name, ...)                                 \
struct name : IEnumeration {                                        \
  enum Type {                                                       \
    JSON_ENUM_VALUES(__VA_ARGS__)                                   \
  };                                                                \
                                                                    \
  Type value;                                                       \
                                                                    \
  name() = default;                                                 \
  constexpr name(Type v) : value(v) {}                              \
  constexpr operator Type() const { return value; }                 \
                                                                    \
  static const QMap<QString, Type> mapping;                         \
  static const QMap<Type, QString> bi_mapping;                      \
                                                                    \
  static Type operator[](const QString& key) {                      \
    auto it = mapping.find(key);                                    \
                                                                    \
    if (it == mapping.end()) {                                      \
      throw std::out_of_range("Invalid enum string key: " +         \
      key.toStdString());                                           \
    }                                                               \
                                                                    \
    return *it;                                                     \
  }                                                                 \
                                                                    \
  static Type operator[](const char* key) {                         \
    return operator[](QString::fromUtf8(key));                      \
  }                                                                 \
                                                                    \
  static const QString& operator[](const Type& type) {              \
    auto it = bi_mapping.find(type);                                \
                                                                    \
    if (it == bi_mapping.end()) {                                   \
      throw std::out_of_range("Invalid enum string key: ");         \
    }                                                               \
                                                                    \
    return *it;                                                     \
  }                                                                 \
};                                                                  \
                                                                    \
const QMap<QString, name::Type> name::mapping = {                   \
  JSON_ENUM_PAIRS(__VA_ARGS__)                                      \
};                                                                  \
                                                                    \
const QMap<name::Type, QString> name::bi_mapping = {                \
  JSON_ENUM_BI_PAIRS(__VA_ARGS__)                                   \
};                                                                  \
