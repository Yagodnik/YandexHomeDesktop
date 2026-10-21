#pragma once

#include <QObject>
#include <QVariantMap>
#include "utils/Settings.h"

class AccountModel;
class ShortcutControlService;

// Presentation adapter for saved settings. It does not listen for shortcuts.
class KeybindSettingsModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ IsAvailable NOTIFY changed)
  Q_PROPERTY(QString accountId READ GetAccountId NOTIFY changed)
  Q_PROPERTY(int revision READ GetRevision NOTIFY changed)
  Q_PROPERTY(QString lastError READ GetLastError NOTIFY lastErrorChanged)
  Q_PROPERTY(QString recordingError READ GetRecordingError NOTIFY recordingChanged)
  Q_PROPERTY(bool recordingReady READ IsRecordingReady NOTIFY recordingChanged)
public:
  explicit KeybindSettingsModel(Settings* settings, AccountModel* account,
                                QObject* parent = nullptr);
  bool IsAvailable() const;
  QString GetAccountId() const;
  int GetRevision() const;
  QString GetLastError() const;
  void SetAccountId(const QString& accountId);
  void SetShortcutControl(ShortcutControlService* control);
  bool IsRecordingReady() const;
  QString GetRecordingError() const;
  Q_INVOKABLE void BeginRecording();
  Q_INVOKABLE void EndRecording();

  Q_INVOKABLE int GetCombination(const QVariantMap& target) const;
  Q_INVOKABLE int MakeCombination(int key, int modifiers) const;
  Q_INVOKABLE QString DisplayCombination(int combination) const;
  Q_INVOKABLE QString ConflictMessage(const QVariantMap& target, int combination) const;
  Q_INVOKABLE bool Save(const QVariantMap& target, int combination, const QString& deviceName,
                        const QString& actionLabel);

signals:
  void recordingChanged();
  void changed();
  void lastErrorChanged();

private:
  ShortcutControlService* shortcut_control_ = nullptr;
  void Reload();
  void SetError(const QString& error);
  static KeybindTarget Target(const QVariantMap& value);
  Settings* settings_;
  QString account_id_;
  std::optional<QList<KeybindAssignment>> bindings_;
  QString last_error_;
  int revision_ = 0;
};
