#include "TitlesProvider.h"

TitlesProvider::TitlesProvider(const QString& path, QObject *parent) : QObject(parent) {
  const auto temp = JsonLoader::LoadRaw(path);

  if (!temp.has_value()) {
    qDebug() << "TitlesProvider: Failed to load JSON data";
    return;
  }

  raw_data_ = temp.value();
}

QString TitlesProvider::GetTitle(const QString &name, const QString &instance) const {
  if (!raw_data_.contains(name)) {
    qWarning() << "Unknown group for" << name << ":" << instance;
    return "???";
  }

  const auto group = raw_data_.value(name).toObject();

  if (!group.contains(instance)) {
    qWarning() << "Unknown instance for" << name << ":" << instance;
    return "???";
  }

  return group.value(instance).toString();
}
