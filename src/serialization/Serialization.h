#pragma once

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <optional>
#include <ranges>
#include <cmath>
#include <utility>
#include <limits>
#include "boost/hana.hpp"
#include "SerializationTypes.h"

namespace Serialization {
  namespace hana = boost::hana;

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

    template<typename T>
    struct is_optional : std::false_type {};

    template<typename T>
    struct is_optional<std::optional<T>> : std::true_type {};

    template<typename T>
    constexpr bool is_optional_v = is_optional<T>::value;
  }

  namespace details {
    template<typename T>
    bool HasJsonType(const QJsonValue& value) {
      if constexpr (std::is_same_v<T, QString> || traits::is_enumeration<T>) {
        return value.isString();
      } else if constexpr (std::is_same_v<T, bool>) {
        return value.isBool();
      } else if constexpr (std::is_integral_v<T>) {
        const auto integer = value.toInteger();
        return value.isDouble() && value.toDouble() == static_cast<double>(integer)
          && std::in_range<T>(integer);
      } else if constexpr (std::is_floating_point_v<T>) {
        const auto number = value.toDouble();
        return value.isDouble() && std::isfinite(number)
          && number >= std::numeric_limits<T>::lowest() && number <= std::numeric_limits<T>::max();
      } else if constexpr (traits::is_qlist_v<T>) {
        if (!value.isArray()) return false;
        const auto array = value.toArray();
        return std::all_of(array.cbegin(), array.cend(), [](const auto& item) {
          return HasJsonType<typename T::value_type>(item);
        });
      } else if constexpr (traits::is_qvmap_v<T>) {
        return value.isObject();
      } else {
        return value.isObject() && TryFrom<T>(value.toObject()).has_value();
      }
    }

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
        return T::AsValue(value.toString());
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
        return T::AsString(value);
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
  std::optional<T> TryFrom(const QJsonObject& data) {
    bool valid = true;
    hana::for_each(hana::accessors<T>(), [&](auto accessor) {
      constexpr auto name = hana::to<const char*>(hana::first(accessor));
      using FieldType = std::decay_t<decltype(hana::second(accessor)(std::declval<T&>()))>;
      valid = valid && details::HasJsonType<FieldType>(data.value(name));
    });
    if (!valid) return std::nullopt;
    return From<T>(data);
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
