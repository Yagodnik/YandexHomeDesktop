#pragma once

#include <QDebug>
#include "JsonLoaderRaw.h"
#include "serialization/Serialization.h"

namespace JsonLoader {
  template<Serialization::Serializable T>
  [[nodiscard]] std::optional<T> Load(const QString& path) {
    const auto temp = LoadRaw(path);

    if (!temp.has_value()) {
      qCritical() << "TitlesProvider: Failed to load JSON data";
      return std::nullopt;
    }

    return Serialization::From<T>(temp.value());
  }
};
