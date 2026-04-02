#include "SettingsStorage.h"

SettingsStorage::SettingsStorage(QObject *parent) : ISettingsStorage(parent) {
}

void SettingsStorage::SaveList(const QString &name, const QStringList &values) {
  settings_.setValue(name, values);
}

QStringList SettingsStorage::GetList(const QString &name) {
  return settings_.value(name).toStringList();
}
