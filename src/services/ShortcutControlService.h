#pragma once

#include "yh/appshortcuts_export.h"
#include "worker/WorkerControlClient.h"
#include "utils/Settings.h"
#include <QTimer>

class APPSHORTCUTS_EXPORT ShortcutControlService final : public QObject {
  Q_OBJECT
public:
  ShortcutControlService(Settings* settings, WorkerLaunchConfiguration configuration,
                         QObject* parent = nullptr);
  void SetAccountId(const QString& account_id);
  void SetEnabled(bool enabled);
  void SetBackground(bool enabled);
  void BeginRecording();
  void EndRecording();
  // Releases this GUI's lease; the worker exits when no foreground clients remain.
  void Shutdown();
  bool IsEnabled() const;
  bool IsBackground() const;
  bool IsBusy() const;
  bool IsRecordingReady() const;
  QJsonObject GetStatus() const;
signals:
  void changed();
  void shutdownFinished();

private:
  enum class Phase { Idle, Starting, Syncing, Stopping, Releasing };
  void Refresh();
  void Pump();
  void Stop();
  void Release();
  void Complete(QJsonObject result);
  Settings* settings_;
  WorkerControlClient worker_;
  QTimer heartbeat_;
  QTimer recording_deadline_;
  QString client_;
  QString account_id_;
  Phase phase_ = Phase::Idle;
  QJsonObject status_;
  int recordings_ = 0;
  bool dirty_ = false;
  bool stop_pending_ = false;
  bool reconciled_ = false;
  bool retry_blocked_ = false;
  QJsonObject failure_;
  bool shutdown_ = false;
  bool recording_ready_ = false;
};
