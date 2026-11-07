#pragma once

#include <QJsonObject>
#include <QVariant>
#include <type_traits>
#include <optional>
#include <boost/hana/define_struct.hpp>
#include "Enumeration.h"

namespace Serialization {
  struct ISerializable {};

  template<typename T>
  concept Serializable = requires(T t) {
    std::is_base_of_v<ISerializable, T>;
    std::is_same_v<T, std::decay_t<T>>;
  };

  template<Serializable T>
  T From(const QJsonObject& data);

  // Require every declared field and its JSON type; unknown fields are ignored.
  template<Serializable T>
  std::optional<T> TryFrom(const QJsonObject& data);

  template<Serializable T>
  QJsonObject To(const T& data);
}

#define JSON_STRUCT(Name, ...)                  \
struct Name : Serialization::ISerializable {    \
  BOOST_HANA_DEFINE_STRUCT(Name, __VA_ARGS__);  \
};
