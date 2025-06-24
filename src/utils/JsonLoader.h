#pragma once

#include <QFile>
#include "serialization/Serialization.h"

namespace JsonLoader {
  template<Serialization::Serializable T>
  [[nodiscard]] std::optional<T> Load(const QString& path) {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
      qWarning() << "Failed to open file: " << path;
      return std::nullopt;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject obj = doc.object();

    return Serialization::From<T>(obj);
  }
};
