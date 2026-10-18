#pragma once

#include <QObject>
#include <QVariantMap>
#include "utils/Settings.h"

class AccountModel;

// Presentation adapter for saved settings. It does not listen for shortcuts.
class KeybindSettingsModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ IsAvailable NOTIFY changed)
  Q_PROPERTY(QString accountId READ GetAccountId NOTIFY changed)
  Q_PROPERTY(int revision READ GetRevision NOTIFY changed)
  Q_PROPERTY(QString lastError READ GetLastError NOTIFY lastErrorChanged)
public:
  explicit KeybindSettingsModel(Settings* settings, AccountModel* account,
                                QObject* parent = nullptr);
  bool IsAvailable() const;
  QString GetAccountId() const;
  int GetRevision() const;
  QString GetLastError() const;
  void SetAccountId(const QString& accountId);

  Q_INVOKABLE int GetCombination(const QVariantMap& target) const;
  Q_INVOKABLE int MakeCombination(int key, int modifiers) const;
  Q_INVOKABLE QString DisplayCombination(int combination) const;
  Q_INVOKABLE QString ConflictMessage(const QVariantMap& target, int combination) const;
  Q_INVOKABLE bool Save(const QVariantMap& target, int combination, const QString& deviceName,
                        const QString& actionLabel);

signals:
  void changed();
  void lastErrorChanged();

private:
  void Reload();
  void SetError(const QString& error);
  static KeybindTarget Target(const QVariantMap& value);
  Settings* settings_;
  QString account_id_;
  std::optional<QList<KeybindAssignment>> bindings_;
  QString last_error_;
  int revision_ = 0;
};
