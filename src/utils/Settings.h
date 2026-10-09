#pragma once

#include "yh/appruntime_export.h"

#include <QSettings>
#include <QHash>

class APPRUNTIME_EXPORT Settings : public QObject {
  Q_OBJECT
  Q_PROPERTY(int currentTheme READ GetCurrentTheme WRITE SetCurrentTheme NOTIFY currentThemeChanged)
  Q_PROPERTY(bool trayModeEnabled READ GetTrayModeEnabled WRITE SetTrayModeEnabled NOTIFY trayModeEnabledChanged)
public:
  explicit Settings(QObject *parent = nullptr, bool temporary = false);
  explicit Settings(const QString& fileName, QObject* parent = nullptr);

  [[nodiscard]] bool GetTrayModeEnabled() const;
  [[nodiscard]] int GetCurrentTheme() const;
  [[nodiscard]] bool GetRestEnabled() const;
  [[nodiscard]] quint16 GetRestPort() const;
  void SetRestEnabled(bool enabled);
  void SetRestPort(quint16 port);
  void Reset();
  static void ResetStoredSettings();
  [[nodiscard]] QStringList GetFavoriteDevices(const QString& accountId) const;
  [[nodiscard]] QStringList GetCollapsedRooms(const QString& accountId) const;
  [[nodiscard]] bool GetFavoritesCollapsed(const QString& accountId) const;
  void SetFavoriteDevices(const QString& accountId, const QStringList& devices);
  void SetCollapsedRooms(const QString& accountId, const QStringList& rooms);
  void SetFavoritesCollapsed(const QString& accountId, bool collapsed);

signals:
  void trayModeEnabledChanged();
  void currentThemeChanged();

public slots:
  void SetCurrentTheme(int theme);
  void SetTrayModeEnabled(bool enabled);

private:
  [[nodiscard]] QVariant ReadAccountValue(const QString& accountId, const QString& name) const;
  void WriteAccountValue(const QString& accountId, const QString& name, const QVariant& value);
  mutable QSettings settings_;
  QHash<QString, QVariant> temporary_account_values_;
  bool temporary_ = false;
  int temporary_theme_ = 0;
  bool temporary_tray_ = false;
  bool temporary_rest_ = false;
  quint16 temporary_rest_port_ = 8766;
};
