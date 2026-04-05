#include "SettingsStorage.h"

SettingsStorage::SettingsStorage(QObject *parent) : ISettingsStorage(parent) {
}

void SettingsStorage::SaveList(const QString &name, const QStringList &values) {
  settings_.setValue(name, values);
}

QStringList SettingsStorage::GetList(const QString &name) {
  return settings_.value(name).toStringList();
}

void SettingsStorage::AddToList(const QString &name, const QString &value) {
  QStringList list = GetList(name);

  if (!list.contains(value)) {
    list.append(value);
  }

  SaveList(name, list);
}

void SettingsStorage::RemoveFromList(const QString &name, const QString &value) {
  QStringList list = GetList(name);

  list.removeIf([value](const QString& el) { return el == value; });

  SaveList(name, list);
}

bool SettingsStorage::Contains(const QString &name, const QString &value) {
  return GetList(name).contains(value);
}
