#include "Settings.h"
#include <QDir>
#include <QCoreApplication>

Settings::Settings(QObject *parent, bool temporary)
  : QObject(parent), settings_("ArtemYagodnik", "YandexHomeDesktop"), temporary_(temporary) {
  if (temporary_) {
    return;
  }

  qInfo() << "Settings: CurrentTheme = " << GetCurrentTheme();
  qInfo() << "Settings: Stored at" << settings_.fileName();

  // Add to autostart
#ifdef Q_OS_WIN32
  settings_.setValue("YandexHomeDesktop", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
  settings_.sync();
#endif
}

bool Settings::GetTrayModeEnabled() const {
  return temporary_ ? temporary_tray_ : settings_.value("trayModeEnabled", false).toBool();
}

int Settings::GetCurrentTheme() const {
  return temporary_ ? temporary_theme_ : settings_.value("currentTheme", 0).toInt();
}

void Settings::Reset() {
  if (temporary_) {
    SetCurrentTheme(0);
    SetTrayModeEnabled(false);
    return;
  }
  ResetStoredSettings();
}

void Settings::ResetStoredSettings() {
  QSettings settings("ArtemYagodnik", "YandexHomeDesktop");
  settings.remove("trayModeEnabled");
  settings.remove("currentTheme");
}

void Settings::SetCurrentTheme(const int theme) {
  if (GetCurrentTheme() == theme) {
    return;
  }

  qDebug() << "Settings::SetCurrentTheme" << theme;
  if (temporary_) { temporary_theme_ = theme; } else { settings_.setValue("currentTheme", theme); }
  emit currentThemeChanged();
}

void Settings::SetTrayModeEnabled(bool enabled) {
  if (GetTrayModeEnabled() == enabled) {
    return;
  }

  qInfo() << "Settings: SetTrayModeEnabled" << enabled;

  if (temporary_) { temporary_tray_ = enabled; } else { settings_.setValue("trayModeEnabled", enabled); }
  emit trayModeEnabledChanged();
}
