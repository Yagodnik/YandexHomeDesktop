#pragma once

#include "ISettingsStorage.h"
#include "utils/Settings.h"

class SettingsStorage : public ISettingsStorage {
public:
  explicit SettingsStorage(QObject* parent = nullptr);

  Q_INVOKABLE void SaveList(const QString &name, const QStringList &values) override;

  Q_INVOKABLE QStringList GetList(const QString &name) override;

private:
  QSettings settings_;
};
