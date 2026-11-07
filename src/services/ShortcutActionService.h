#pragma once

#include "yh/appshortcuts_export.h"
#include "DeviceService.h"
#include "ScenarioService.h"
#include "utils/KeybindSettings.h"
#include <QHash>
#include <QPointer>

// One fresh read and one validated write per activation. Operations for the same
// device/scenario cannot overlap, and reset invalidates callbacks before removing contexts.
class APPSHORTCUTS_EXPORT ShortcutActionService final : public QObject {
  Q_OBJECT
public:
  explicit ShortcutActionService(DeviceService* devices, QObject* parent = nullptr,
                                 int timeout_ms = 30'000);
  ShortcutActionService(DeviceService* devices, ScenarioService* scenarios, QObject* parent,
                        int timeout_ms = 30'000);
  void Activate(const KeybindTarget& target);
  void Reset();
signals:
  void finished(const QString& target_id, const QString& error_code);

private:
  using OperationKey = QPair<bool, QString>; // Scenario flag and stable entity ID.
  struct Operation;
  void Finish(const OperationKey& key, Operation* operation, const QString& error);
  QPointer<DeviceService> devices_;
  QPointer<ScenarioService> scenarios_;
  QHash<OperationKey, QPointer<Operation>> pending_;
  int timeout_ms_;
};
