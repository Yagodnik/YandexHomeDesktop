#pragma once

#include <QSettings>
#include <QStringList>

// This operation is intentionally independent of QApplication and all GUI services.
inline bool IsTrayRecoveryRequest(const QStringList& arguments) {
  return arguments.size() == 2 && arguments.at(1) == QStringLiteral("--disable-tray");
}

inline bool DisableTrayForRecovery(QSettings& settings) {
  settings.setValue(QStringLiteral("trayModeEnabled"), false);
  settings.sync();
  return settings.status() == QSettings::NoError;
}
