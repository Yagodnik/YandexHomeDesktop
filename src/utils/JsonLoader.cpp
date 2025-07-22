#include "JsonLoader.h"

std::optional<QJsonObject> JsonLoader::LoadRaw(const QString &path)  {
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open file: " << path;
    return std::nullopt;
  }

  const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

  return doc.object();
}