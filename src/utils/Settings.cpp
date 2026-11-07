#include "Settings.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSet>
#include <algorithm>
#include <tuple>
#include <expected>
#include "serialization/Serialization.h"

Settings::Settings(QObject *parent, bool temporary)
  : QObject(parent), settings_("ArtemYagodnik", "YandexHomeDesktop"), temporary_(temporary) {
  if (temporary_) {
    return;
  }

  if (GetTrayModeEnabled() && GetNotchEnabled()) settings_.setValue("notchEnabled", false);
  qInfo() << "Settings: CurrentTheme = " << GetCurrentTheme();
  qInfo() << "Settings: Stored at" << settings_.fileName();

}

Settings::Settings(const QString& fileName, QObject* parent)
  : QObject(parent), settings_(fileName, QSettings::IniFormat) {
  if (GetTrayModeEnabled() && GetNotchEnabled()) settings_.setValue("notchEnabled", false);
}

namespace {
constexpr int KeybindFormatVersion = 2;
JSON_STRUCT(KeybindDocument,
  (int, version),
  (QList<KeybindAssignment>, bindings)
);

QString AccountKey(const QString& accountId, const QString& name) {
  const auto id = QCryptographicHash::hash(accountId.toUtf8(), QCryptographicHash::Sha256).toHex();
  return "accounts/" + QString::fromLatin1(id) + "/" + name;
}

JSON_STRUCT(AppPreferences,
  (int, theme), (bool, tray_mode), (bool, show_keybinds)
);
JSON_STRUCT(AccountPreferences,
  (QString, id), (QStringList, favorite_devices), (QStringList, collapsed_rooms),
  (bool, favorites_collapsed), (QList<KeybindAssignment>, keybinds)
);
JSON_STRUCT(SettingsBackup,
  (int, version), (AppPreferences, app), (AccountPreferences, account)
);

bool ValidBindings(const QList<KeybindAssignment>& bindings) {
  QSet<int> combinations;
  QList<KeybindTarget> targets;
  for (const auto& binding : bindings) {
    if (!Settings::IsValidKeybindTarget(binding.target) || binding.combination == 0
        || Settings::NormalizeKeyCombination(binding.combination & ~Qt::KeyboardModifierMask,
          binding.combination & Qt::KeyboardModifierMask) != binding.combination
        || combinations.contains(binding.combination)) return false;
    if (std::any_of(targets.cbegin(), targets.cend(), [&](const auto& target) {
      return Settings::SameKeybindTarget(target, binding.target);
    })) return false;
    combinations.insert(binding.combination);
    targets.append(binding.target);
  }
  return true;
}

bool DeviceBindingsOnly(const QList<KeybindAssignment>& bindings) {
  return std::all_of(bindings.cbegin(), bindings.cend(), [](const auto& binding) {
    return binding.target.scenario_id.isEmpty();
  });
}

bool ValidIds(const QStringList& ids) {
  QSet<QString> unique;
  for (const auto& id : ids) {
    if (id.isEmpty() || unique.contains(id)) return false;
    unique.insert(id);
  }
  return true;
}

// Version 1 targets contain device fields only. Add the explicit empty scenario
// identifier before strict decoding; never repair malformed version 2 input.
QJsonArray UpgradeDeviceBindings(QJsonArray bindings) {
  for (qsizetype index = 0; index < bindings.size(); ++index) {
    if (!bindings[index].isObject()) continue;
    auto binding = bindings[index].toObject();
    if (!binding["target"].isObject()) continue;
    auto target = binding["target"].toObject();
    if (!target.contains("scenario_id")) target.insert("scenario_id", "");
    binding.insert("target", target);
    bindings[index] = binding;
  }
  return bindings;
}

std::expected<SettingsBackup, Settings::ImportResult> ReadBackup(const QString& accountId,
                                                               const QByteArray& data) {
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError || !document.isObject())
    return std::unexpected(Settings::ImportResult::InvalidData);
  auto object = document.object();
  if (object["version"] == 1 && object["account"].isObject()) {
    auto account = object["account"].toObject();
    if (account["keybinds"].isArray()) account["keybinds"] = UpgradeDeviceBindings(account["keybinds"].toArray());
    object["account"] = account;
  }
  const auto backup = Serialization::TryFrom<SettingsBackup>(object);
  if (!backup || (backup->version != 1 && backup->version != KeybindFormatVersion) || backup->app.theme < 0 || backup->app.theme > 1
      || backup->account.id.isEmpty() || !ValidBindings(backup->account.keybinds)
      || (backup->version == 1 && !DeviceBindingsOnly(backup->account.keybinds))
      || !ValidIds(backup->account.favorite_devices) || !ValidIds(backup->account.collapsed_rooms))
    return std::unexpected(Settings::ImportResult::InvalidData);
  if (accountId.isEmpty() || backup->account.id != accountId)
    return std::unexpected(Settings::ImportResult::AccountMismatch);
  return *backup;
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

bool Settings::SameKeybindTarget(const KeybindTarget& left, const KeybindTarget& right) {
  return std::tie(left.device_id, left.capability_type, left.instance, left.action, left.value, left.scenario_id)
    == std::tie(right.device_id, right.capability_type, right.instance, right.action, right.value, right.scenario_id);
}

bool Settings::IsValidKeybindTarget(const KeybindTarget& target) {
  if (!target.scenario_id.isEmpty())
    return target.device_id.isEmpty() && target.capability_type.isEmpty() && target.instance.isEmpty()
      && target.action == "run" && target.value.isEmpty();
  if (target.device_id.isEmpty()) return false;
  const auto& type = target.capability_type;
  if (type == "devices.capabilities.on_off")
    return target.instance == "on" && target.action == "toggle" && target.value.isEmpty();
  if (type == "devices.capabilities.toggle")
    return !target.instance.isEmpty() && target.action == "toggle" && target.value.isEmpty();
  if (type == "devices.capabilities.range")
    return !target.instance.isEmpty() && (target.action == "increase" || target.action == "decrease") && target.value.isEmpty();
  if (type == "devices.capabilities.mode")
    return !target.instance.isEmpty() && target.action == "set_mode" && !target.value.isEmpty();
  if (type != "devices.capabilities.color_setting" || !target.instance.isEmpty()) return false;
  if (target.action == "set_scene") return !target.value.isEmpty();
  if (target.action == "set_temperature") {
    bool valid = false;
    const auto temperature = target.value.toInt(&valid);
    return valid && temperature > 0 && QString::number(temperature) == target.value;
  }
  if (target.action == "set_color") {
    if (target.value.size() != 7 || !target.value.startsWith('#')) return false;
    return std::all_of(target.value.cbegin() + 1, target.value.cend(), [](QChar character) {
      return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f');
    });
  }
  return false;
}

int Settings::NormalizeKeyCombination(int key, int modifiers) {
  constexpr int allowed = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
  modifiers &= allowed;
  if (key >= 'a' && key <= 'z') key -= 'a' - 'A';
  if (key == Qt::Key_Enter) key = Qt::Key_Return;
  if (key == Qt::Key_Backtab) { key = Qt::Key_Tab; modifiers |= Qt::ShiftModifier; }
  if (key < Qt::Key_Space || key >= Qt::Key_unknown || key == Qt::Key_Shift || key == Qt::Key_Control
      || key == Qt::Key_Alt || key == Qt::Key_Meta || key == Qt::Key_AltGr || key == Qt::Key_Escape
      || key == Qt::Key_Backspace || key == Qt::Key_Delete) return 0;
  if (key == Qt::Key_Tab && !(modifiers & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) return 0;
  return key | modifiers;
}

std::optional<QList<KeybindAssignment>> Settings::GetKeybinds(const QString& accountId) const {
  if (!temporary_) settings_.sync();
  const auto stored = ReadAccountValue(accountId, "keybinds");
  if (!stored.isValid()) return QList<KeybindAssignment>{};
  if (stored.metaType().id() != QMetaType::QString) return std::nullopt;
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(stored.toString().toUtf8(), &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) return std::nullopt;
  auto object = document.object();
  if (object["version"] == 1 && object["bindings"].isArray())
    object["bindings"] = UpgradeDeviceBindings(object["bindings"].toArray());
  const auto decoded = Serialization::TryFrom<KeybindDocument>(object);
  if (!decoded || (decoded->version != 1 && decoded->version != KeybindFormatVersion) || !ValidBindings(decoded->bindings)
      || (decoded->version == 1 && !DeviceBindingsOnly(decoded->bindings))) return std::nullopt;
  return decoded->bindings;
}

Settings::KeybindSaveResult Settings::SaveKeybind(const QString& accountId, const KeybindTarget& target,
    int combination, const QString& deviceName, const QString& actionLabel) {
  if (accountId.isEmpty() || !IsValidKeybindTarget(target)) return KeybindSaveResult::Invalid;
  if (combination != 0 && NormalizeKeyCombination(combination & ~Qt::KeyboardModifierMask,
      combination & Qt::KeyboardModifierMask) != combination) return KeybindSaveResult::Invalid;
  auto bindings = GetKeybinds(accountId);
  if (!bindings) return KeybindSaveResult::StorageError;
  for (const auto& binding : *bindings) {
    if (combination != 0 && binding.combination == combination && !SameKeybindTarget(binding.target, target))
      return KeybindSaveResult::Conflict;
  }
  bindings->removeIf([&](const auto& binding) { return SameKeybindTarget(binding.target, target); });
  if (combination != 0) {
    KeybindAssignment binding{};
    binding.target = target;
    binding.combination = combination;
    binding.device_name = deviceName;
    binding.action_label = actionLabel;
    bindings->append(binding);
  }
  KeybindDocument document{};
  document.version = KeybindFormatVersion;
  document.bindings = *bindings;
  const auto serialized = QString::fromUtf8(QJsonDocument(Serialization::To(document)).toJson(QJsonDocument::Compact));
  const auto previous = ReadAccountValue(accountId, "keybinds");
  WriteAccountValue(accountId, "keybinds", serialized);
  if (!temporary_ && settings_.status() != QSettings::NoError) {
    const auto key = AccountKey(accountId, "keybinds");
    if (previous.isValid()) settings_.setValue(key, previous); else settings_.remove(key);
    return KeybindSaveResult::StorageError;
  }
  emit keybindsChanged(accountId);
  return KeybindSaveResult::Saved;
}

std::optional<QByteArray> Settings::ExportSettings(const QString& accountId) const {
  if (accountId.isEmpty()) return std::nullopt;
  const auto keybinds = GetKeybinds(accountId);
  if (!keybinds) return std::nullopt;
  SettingsBackup backup{};
  backup.version = KeybindFormatVersion;
  backup.app.theme = GetCurrentTheme();
  backup.app.tray_mode = GetTrayModeEnabled();
  backup.app.show_keybinds = GetShowKeybinds();
  backup.account.id = accountId;
  backup.account.favorite_devices = GetFavoriteDevices(accountId);
  backup.account.collapsed_rooms = GetCollapsedRooms(accountId);
  backup.account.favorites_collapsed = GetFavoritesCollapsed(accountId);
  backup.account.keybinds = *keybinds;
  return QJsonDocument(Serialization::To(backup)).toJson(QJsonDocument::Indented);
}

Settings::ImportResult Settings::CheckImport(const QString& accountId, const QByteArray& data) const {
  const auto backup = ReadBackup(accountId, data);
  return backup ? ImportResult::Imported : backup.error();
}

Settings::ImportResult Settings::ImportSettings(const QString& accountId, const QByteArray& data) {
  const auto backup = ReadBackup(accountId, data);
  if (!backup) return backup.error();
  KeybindDocument keybinds{};
  keybinds.version = KeybindFormatVersion;
  keybinds.bindings = backup->account.keybinds;
  const auto serialized = QString::fromUtf8(QJsonDocument(Serialization::To(keybinds)).toJson(QJsonDocument::Compact));
  const auto oldTheme = GetCurrentTheme();
  const auto oldTray = GetTrayModeEnabled();
  const auto oldNotch = GetNotchEnabled();
  const auto oldShow = GetShowKeybinds();
  QHash<QString, QVariant> values{
    {AccountKey(accountId, "favoriteDevices"), backup->account.favorite_devices},
    {AccountKey(accountId, "collapsedRooms"), backup->account.collapsed_rooms},
    {AccountKey(accountId, "favoritesCollapsed"), backup->account.favorites_collapsed},
    {AccountKey(accountId, "keybinds"), serialized}
  };
  if (temporary_) {
    for (auto it = values.cbegin(); it != values.cend(); ++it) temporary_account_values_.insert(it.key(), it.value());
    temporary_theme_ = backup->app.theme;
    temporary_tray_ = backup->app.tray_mode;
    if (backup->app.tray_mode) temporary_notch_ = false;
    temporary_show_keybinds_ = backup->app.show_keybinds;
  } else {
    values.insert("currentTheme", backup->app.theme);
    values.insert("trayModeEnabled", backup->app.tray_mode);
    if (backup->app.tray_mode) values.insert("notchEnabled", false);
    values.insert("showKeybinds", backup->app.show_keybinds);
    QHash<QString, QVariant> previous;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
      previous.insert(it.key(), settings_.value(it.key()));
      settings_.setValue(it.key(), it.value());
    }
    settings_.sync();
    if (settings_.status() != QSettings::NoError) {
      for (auto it = previous.cbegin(); it != previous.cend(); ++it) {
        if (it.value().isValid()) settings_.setValue(it.key(), it.value());
        else settings_.remove(it.key());
      }
      settings_.sync();
      return ImportResult::StorageError;
    }
  }
  if (oldTheme != backup->app.theme) emit currentThemeChanged();
  if (oldNotch && backup->app.tray_mode) emit notchEnabledChanged();
  if (oldTray != backup->app.tray_mode) emit trayModeEnabledChanged();
  if (oldShow != backup->app.show_keybinds) emit showKeybindsChanged();
  emit keybindsChanged(accountId);
  emit accountPreferencesChanged(accountId);
  return ImportResult::Imported;
}

bool Settings::GetTrayModeEnabled() const {
  return temporary_ ? temporary_tray_ : settings_.value("trayModeEnabled", false).toBool();
}

bool Settings::GetNotchEnabled() const {
  return temporary_ ? temporary_notch_ : settings_.value("notchEnabled", false).toBool();
}

void Settings::SetNotchEnabled(bool enabled) {
  const bool oldNotch = GetNotchEnabled();
  const bool oldTray = GetTrayModeEnabled();
  if (oldNotch == enabled && !(enabled && oldTray)) return;
  if (temporary_) {
    temporary_notch_ = enabled;
    if (enabled) temporary_tray_ = false;
  } else {
    settings_.setValue("notchEnabled", enabled);
    if (enabled) settings_.setValue("trayModeEnabled", false);
  }
  if (oldTray && enabled) emit trayModeEnabledChanged();
  if (oldNotch != enabled) emit notchEnabledChanged();
}

int Settings::GetCurrentTheme() const {
  return temporary_ ? temporary_theme_ : settings_.value("currentTheme", 0).toInt();
}

bool Settings::GetShowKeybinds() const {
  return temporary_ ? temporary_show_keybinds_ : settings_.value("showKeybinds", true).toBool();
}

bool Settings::GetRestEnabled() const {
  if (temporary_) return temporary_rest_;
  settings_.sync();
  return settings_.value("rest/enabled", false).toBool();
}

quint16 Settings::GetRestPort() const {
  if (temporary_) return temporary_rest_port_;
  settings_.sync();
  const auto port = settings_.value("rest/port", 8765).toUInt();
  return port > 0 && port <= 65535 ? static_cast<quint16>(port) : 8765;
}

void Settings::SetRestEnabled(bool enabled) {
  if (temporary_) temporary_rest_ = enabled;
  else { settings_.setValue("rest/enabled", enabled); settings_.sync(); }
}

void Settings::SetRestPort(quint16 port) {
  if (temporary_) temporary_rest_port_ = port;
  else { settings_.setValue("rest/port", port); settings_.sync(); }
}

quint16 Settings::GetMcpPort() const {
  if (temporary_) return temporary_mcp_port_;
  settings_.sync();
  const auto port = settings_.value("mcp/port", 8767).toUInt();
  return port > 0 && port <= 65535 ? static_cast<quint16>(port) : 8767;
}

void Settings::SetMcpPort(quint16 port) {
  if (temporary_) temporary_mcp_port_ = port;
  else { settings_.setValue("mcp/port", port); settings_.sync(); }
}

bool Settings::GetShortcutsEnabled() const {
  if (temporary_) return temporary_shortcuts_;
  settings_.sync();
  return settings_.value("shortcuts/enabled", true).toBool();
}

bool Settings::GetShortcutsBackground() const {
  if (temporary_) return temporary_shortcuts_background_;
  settings_.sync();
  return settings_.value("shortcuts/background", false).toBool();
}

void Settings::SetShortcutsEnabled(bool enabled) {
  if (temporary_) temporary_shortcuts_ = enabled;
  else { settings_.setValue("shortcuts/enabled", enabled); settings_.sync(); }
}

void Settings::SetShortcutsBackground(bool enabled) {
  if (temporary_) temporary_shortcuts_background_ = enabled;
  else { settings_.setValue("shortcuts/background", enabled); settings_.sync(); }
}

void Settings::Reset() {
  if (temporary_) {
    temporary_account_values_.clear();
    SetCurrentTheme(0);
    SetTrayModeEnabled(false);
    SetNotchEnabled(false);
    SetShowKeybinds(true);
    emit keybindsChanged({});
    return;
  }
  SetNotchEnabled(false);
  settings_.remove("notchEnabled");
  settings_.remove("trayModeEnabled");
  settings_.remove("currentTheme");
  SetShowKeybinds(true);
  settings_.remove("showKeybinds");
  settings_.remove("accounts");
  emit keybindsChanged({});
}

void Settings::ResetStoredSettings() {
  QSettings settings("ArtemYagodnik", "YandexHomeDesktop");
  settings.remove("notchEnabled");
  settings.remove("trayModeEnabled");
  settings.remove("currentTheme");
  settings.remove("showKeybinds");
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
  const bool oldTray = GetTrayModeEnabled();
  const bool oldNotch = GetNotchEnabled();
  if (oldTray == enabled && !(enabled && oldNotch)) return;
  if (temporary_) {
    temporary_tray_ = enabled;
    if (enabled) temporary_notch_ = false;
  } else {
    settings_.setValue("trayModeEnabled", enabled);
    if (enabled) settings_.setValue("notchEnabled", false);
  }
  if (oldNotch && enabled) emit notchEnabledChanged();
  if (oldTray != enabled) emit trayModeEnabledChanged();
}

void Settings::SetShowKeybinds(bool visible) {
  if (GetShowKeybinds() == visible) {
    return;
  }
  if (temporary_) { temporary_show_keybinds_ = visible; }
  else { settings_.setValue("showKeybinds", visible); }
  emit showKeybindsChanged();
}
