#pragma once

#include <QFile>
#include "serialization/Serialization.h"

namespace JsonLoader {
  [[nodiscard]] std::optional<QJsonObject> LoadRaw(const QString& path);

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
