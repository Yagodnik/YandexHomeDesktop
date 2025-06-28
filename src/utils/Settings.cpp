#include "Settings.h"

Settings::Settings(QObject *parent)
  : QObject(parent), settings_("ArtemYagodnik", "YandexHomeDesktop") {
  qDebug() << "CurrentTheme" << GetCurrentTheme();
  qDebug() << "Settings initialized:" << settings_.fileName();
}

bool Settings::GetTrayModeEnabled() const {
  return settings_.value("trayModeEnabled", false).toBool();
}

int Settings::GetCurrentTheme() const {
  return settings_.value("currentTheme", 0).toInt();
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

  qDebug() << "Settings::SetTrayModeEnabled" << enabled;

  settings_.setValue("trayModeEnabled", enabled);
  emit trayModeEnabledChanged();
}


