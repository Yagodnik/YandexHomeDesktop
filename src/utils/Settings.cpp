#include "Settings.h"
#include <QDir>
#include <QCoreApplication>
#include <QCryptographicHash>

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

Settings::Settings(const QString& fileName, QObject* parent)
  : QObject(parent), settings_(fileName, QSettings::IniFormat) {}

namespace {
QString AccountKey(const QString& accountId, const QString& name) {
  const auto id = QCryptographicHash::hash(accountId.toUtf8(), QCryptographicHash::Sha256).toHex();
  return "accounts/" + QString::fromLatin1(id) + "/" + name;
}
}

QVariant Settings::ReadAccountValue(const QString& accountId, const QString& name) const {
  if (accountId.isEmpty()) { return {}; }
  const auto key = AccountKey(accountId, name);
  return temporary_ ? temporary_account_values_.value(key) : settings_.value(key);
}

void Settings::WriteAccountValue(const QString& accountId, const QString& name, const QVariant& value) {
  if (accountId.isEmpty()) { return; }
  const auto key = AccountKey(accountId, name);
  if (temporary_) {
    temporary_account_values_.insert(key, value);
  } else {
    settings_.setValue(key, value);
    settings_.sync();
  }
}

QStringList Settings::GetFavoriteDevices(const QString& accountId) const {
  return ReadAccountValue(accountId, "favoriteDevices").toStringList();
}

QStringList Settings::GetCollapsedRooms(const QString& accountId) const {
  return ReadAccountValue(accountId, "collapsedRooms").toStringList();
}

void Settings::SetFavoriteDevices(const QString& accountId, const QStringList& devices) {
  WriteAccountValue(accountId, "favoriteDevices", devices);
}

void Settings::SetCollapsedRooms(const QString& accountId, const QStringList& rooms) {
  WriteAccountValue(accountId, "collapsedRooms", rooms);
}

bool Settings::GetFavoritesCollapsed(const QString& accountId) const {
  return ReadAccountValue(accountId, "favoritesCollapsed").toBool();
}

void Settings::SetFavoritesCollapsed(const QString& accountId, bool collapsed) {
  WriteAccountValue(accountId, "favoritesCollapsed", collapsed);
}

bool Settings::GetTrayModeEnabled() const {
  return temporary_ ? temporary_tray_ : settings_.value("trayModeEnabled", false).toBool();
}

int Settings::GetCurrentTheme() const {
  return temporary_ ? temporary_theme_ : settings_.value("currentTheme", 0).toInt();
}

void Settings::Reset() {
  if (temporary_) {
    temporary_account_values_.clear();
    SetCurrentTheme(0);
    SetTrayModeEnabled(false);
    return;
  }
  settings_.remove("trayModeEnabled");
  settings_.remove("currentTheme");
  settings_.remove("accounts");
}

void Settings::ResetStoredSettings() {
  QSettings settings("ArtemYagodnik", "YandexHomeDesktop");
  settings.remove("trayModeEnabled");
  settings.remove("currentTheme");
  settings.remove("accounts");
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
