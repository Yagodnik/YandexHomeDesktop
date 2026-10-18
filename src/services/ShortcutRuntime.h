#pragma once

#include "yh/appshortcuts_export.h"
#include "ShortcutActionService.h"
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <functional>

// Native registration is the only platform-specific dependency. The worker is
// the sole owner; tests supply a registrar without touching OS shortcuts.
class APPSHORTCUTS_EXPORT IShortcutRegistrar {
public:
  virtual ~IShortcutRegistrar() = default;
  virtual bool Register(int combination, std::function<void()> activated) = 0;
  virtual bool Clear() = 0;
};

class APPSHORTCUTS_EXPORT ShortcutRuntime final : public QObject {
  Q_OBJECT
public:
  explicit ShortcutRuntime(IShortcutRegistrar* registrar, ShortcutActionService* actions,
                           QString account_id, QObject* parent = nullptr, int lease_ms = 10'000);
  ~ShortcutRuntime() override;
  QJsonObject Handle(const QJsonObject& request);
  QJsonObject Status() const;
  void Stop();
signals:
  void stopRequested();

private:
  void Expire();
  void Apply();
  struct Lease {
    qint64 expires;
    bool recording;
  };
  IShortcutRegistrar* registrar_;
  ShortcutActionService* actions_;
  QString account_id_;
  QElapsedTimer clock_;
  QTimer timer_;
  QHash<QString, Lease> leases_;
  QList<KeybindAssignment> bindings_;
  QJsonArray encoded_bindings_;
  QJsonArray conflicts_;
  int lease_ms_;
  int registered_ = 0;
  int conflict_count_ = 0;
  int completed_ = 0;
  int failed_ = 0;
  bool background_ = false;
  bool paused_ = false;
  bool running_ = true;
  bool applied_ = false;
  bool released_ = true;
  quint64 generation_ = 0;
  QString action_error_;
};
