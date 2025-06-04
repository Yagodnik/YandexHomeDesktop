#pragma once

#include <QJsonValue>
#include <QJsonObject>
#include "boost/hana.hpp"

namespace Serialization {
  namespace hana = boost::hana;

  template<typename T>
  concept Serializable = requires(T t) {
    std::is_same_v<T, std::decay_t<T>>;
  };

  template<Serializable T>
  T From(const QJsonObject& data);

  template<Serializable T>
  QJsonObject To(const T& data);

  namespace details {
    template<typename T>
    T CastFromJsonValue(const QJsonValue& value) {
      if constexpr (std::is_same_v<T, QString>) {
        return value.toString();
      } else if constexpr (std::is_integral_v<T>) {
        return value.toInteger();
      } else if constexpr (std::is_floating_point_v<T>) {
        return value.toDouble();
      } else {
        return From<T>(value.toObject());
      }
    }

    template<typename T>
    auto CastToJsonValue(const T& value) {
      if constexpr (std::is_same_v<T, QString>) {
        return value;
      } else if constexpr (std::is_integral_v<T>) {
        return static_cast<qint64>(value);
      } else if constexpr (std::is_floating_point_v<T>) {
        return static_cast<double>(value);
      } else {
        return To<T>(value);
      }
    }
  }

  template<Serializable T>
  T From(const QJsonObject& data) {
    T result;

    hana::for_each(hana::accessors<T>(), [&](auto accessor) {
      constexpr auto name = hana::to<const char*>(hana::first(accessor));
      auto& field_ref = hana::second(accessor)(result);
      using FieldType = std::decay_t<decltype(field_ref)>;

      if (data.contains(name)) {
        const auto value = data.value(name);
        field_ref = details::CastFromJsonValue<FieldType>(value);
      }
    });

    return result;
  }

  template<Serializable T>
  QJsonObject To(const T& data) {
    QJsonObject result;

    hana::for_each(hana::accessors<T>(), [&](auto accessor) {
      constexpr auto name = hana::to<const char*>(hana::first(accessor));
      const auto& field = hana::second(accessor)(data);
      using FieldType = std::decay_t<decltype(field)>;

      result[name] = details::CastToJsonValue<FieldType>(field);
    });

    return result;
  }
}

#define JSON_STRUCT(Name, ...) \
struct Name { \
BOOST_HANA_DEFINE_STRUCT(Name, __VA_ARGS__); \
};

