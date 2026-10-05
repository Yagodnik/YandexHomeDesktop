#pragma once

#include <QJsonObject>
#include <QString>
#include <optional>

namespace JsonLoader {
  [[nodiscard]] std::optional<QJsonObject> LoadRaw(const QString& path);
}
