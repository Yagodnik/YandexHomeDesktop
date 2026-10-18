#pragma once

#include "yh/appruntime_export.h"

#include <QSettings>
#include <QHash>
#include <optional>
#include "KeybindSettings.h"

class APPRUNTIME_EXPORT Settings : public QObject {
  Q_OBJECT
  Q_PROPERTY(int currentTheme READ GetCurrentTheme WRITE SetCurrentTheme NOTIFY currentThemeChanged)
  Q_PROPERTY(bool trayModeEnabled READ GetTrayModeEnabled WRITE SetTrayModeEnabled NOTIFY trayModeEnabledChanged)
  Q_PROPERTY(bool showKeybinds READ GetShowKeybinds WRITE SetShowKeybinds NOTIFY showKeybindsChanged)
public:
  explicit Settings(QObject *parent = nullptr, bool temporary = false);
  explicit Settings(const QString& fileName, QObject* parent = nullptr);

  [[nodiscard]] bool GetTrayModeEnabled() const;
  [[nodiscard]] int GetCurrentTheme() const;
  [[nodiscard]] bool GetShowKeybinds() const;
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
  enum class KeybindSaveResult { Saved, Conflict, Invalid, StorageError };
  [[nodiscard]] std::optional<QList<KeybindAssignment>> GetKeybinds(const QString& accountId) const;
  [[nodiscard]] KeybindSaveResult SaveKeybind(const QString& accountId, const KeybindTarget& target,
    int combination, const QString& deviceName, const QString& actionLabel);
  [[nodiscard]] static bool SameKeybindTarget(const KeybindTarget& left, const KeybindTarget& right);
  [[nodiscard]] static bool IsValidKeybindTarget(const KeybindTarget& target);
  [[nodiscard]] static int NormalizeKeyCombination(int key, int modifiers);
  enum class ImportResult { Imported, InvalidData, AccountMismatch, StorageError };
  [[nodiscard]] std::optional<QByteArray> ExportSettings(const QString& accountId) const;
  [[nodiscard]] ImportResult CheckImport(const QString& accountId, const QByteArray& data) const;
  [[nodiscard]] ImportResult ImportSettings(const QString& accountId, const QByteArray& data);

signals:
  void trayModeEnabledChanged();
  void currentThemeChanged();
  void showKeybindsChanged();
  void keybindsChanged(const QString& accountId);
  void accountPreferencesChanged(const QString& accountId);

public slots:
  void SetCurrentTheme(int theme);
  void SetTrayModeEnabled(bool enabled);
  void SetShowKeybinds(bool visible);

private:
  [[nodiscard]] QVariant ReadAccountValue(const QString& accountId, const QString& name) const;
  void WriteAccountValue(const QString& accountId, const QString& name, const QVariant& value);
  mutable QSettings settings_;
  QHash<QString, QVariant> temporary_account_values_;
  bool temporary_ = false;
  int temporary_theme_ = 0;
  bool temporary_tray_ = false;
  bool temporary_show_keybinds_ = true;
  bool temporary_rest_ = false;
  quint16 temporary_rest_port_ = 8766;
};
