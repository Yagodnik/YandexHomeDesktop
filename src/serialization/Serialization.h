#pragma once

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <ranges>
#include "boost/hana.hpp"
#include "Enumeration.h"

namespace Serialization {
  namespace hana = boost::hana;

  struct ISerializable {};

  template<typename T>
  concept Serializable = requires(T t) {
    std::is_base_of_v<ISerializable, T>;
    std::is_same_v<T, std::decay_t<T>>;
  };

  template<Serializable T>
  T From(const QJsonObject& data);

  template<Serializable T>
  QJsonObject To(const T& data);

  namespace traits {
    template<typename T>
    constexpr bool is_enumeration = std::is_base_of_v<IEnumeration, T>;

    template<typename T>
    struct is_qlist : std::false_type {};

    template<typename T>
    struct is_qlist<QList<T>> : std::true_type {};

    template<typename T>
    constexpr bool is_qlist_v = is_qlist<T>::value;

    template<typename T>
    struct is_qvmap : std::false_type {};

    template<>
    struct is_qvmap<QVariantMap> : std::true_type {};

    template<typename T>
    constexpr bool is_qvmap_v = is_qvmap<T>::value;
  }

  namespace details {
    template<typename T>
    T CastFromJsonValue(const QJsonValue& value) {
      if constexpr (std::is_same_v<T, QString>) {
        return value.toString();
      } else if constexpr (std::is_same_v<T, bool>) {
        return value.toBool();
      } else if constexpr (std::is_integral_v<T>) {
        return value.toInteger();
      } else if constexpr (std::is_floating_point_v<T>) {
        return value.toDouble();
      } else if constexpr (traits::is_enumeration<T>) {
        return T::operator[](value.toString());
      } else if constexpr (traits::is_qlist_v<T>) {
        using Element = typename T::value_type;
        const auto arr = value.toArray();

        auto a = arr | std::views::transform([](const QJsonValue& item) {
          return CastFromJsonValue<Element>(item);
        });

        return QList<Element>(a.begin(), a.end());
      } else if constexpr (traits::is_qvmap_v<T>) {
        return value.toObject().toVariantMap();
      } else {
        return From<T>(value.toObject());
      }
    }

    template<typename T>
    auto CastToJsonValue(const T& value) {
      if constexpr (std::is_same_v<T, QString>) {
        return value;
      } else if constexpr (std::is_same_v<T, bool>) {
        return static_cast<bool>(value);
      } else if constexpr (std::is_integral_v<T>) {
        return static_cast<qint64>(value);
      } else if constexpr (std::is_floating_point_v<T>) {
        return static_cast<double>(value);
      } else if constexpr (traits::is_enumeration<T>) {
        return T::operator[](value);
      } else if constexpr (traits::is_qlist_v<T>) {
        QJsonArray arr;

        std::ranges::for_each(value, [&arr](const auto& item) {
          arr.append(CastToJsonValue(item));
        });

        return QJsonValue(arr);
      } else if constexpr (traits::is_qvmap_v<T>) {
        return QJsonObject::fromVariantMap(value);
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

#define JSON_STRUCT(Name, ...)                  \
struct Name : Serialization::ISerializable {    \
  BOOST_HANA_DEFINE_STRUCT(Name, __VA_ARGS__);  \
};                                              \

