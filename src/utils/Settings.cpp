#include "Settings.h"
#include <QDir>
#include <QCoreApplication>

Settings::Settings(QObject *parent)
  : QObject(parent), settings_("ArtemYagodnik", "YandexHomeDesktop") {

  qInfo() << "Settings: CurrentTheme = " << GetCurrentTheme();
  qInfo() << "Settings: Stored at" << settings_.fileName();

  // Add to autostart
#ifdef Q_OS_WIN32
  settings_.setValue("YandexHomeDesktop", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
  settings_.sync();
#endif
}

bool Settings::GetTrayModeEnabled() const {
  return settings_.value("trayModeEnabled", false).toBool();
}

int Settings::GetCurrentTheme() const {
  return settings_.value("currentTheme", 0).toInt();
}

void Settings::Reset() {
  settings_.remove("trayModeEnabled");
  settings_.remove("currentTheme");
}

void Settings::SetCurrentTheme(const int theme) {
  if (GetCurrentTheme() == theme) {
    return;
  }

  qDebug() << "Settings::SetCurrentTheme" << theme;
  settings_.setValue("currentTheme", theme);
  emit currentThemeChanged();
}

void Settings::SetTrayModeEnabled(bool enabled) {
  if (GetTrayModeEnabled() == enabled) {
    return;
  }

  qInfo() << "Settings: SetTrayModeEnabled" << enabled;

  settings_.setValue("trayModeEnabled", enabled);
  emit trayModeEnabledChanged();
}


