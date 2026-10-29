#pragma once
#include <QObject>
#include <QTimer>
#include "worker/WorkerControlClient.h"
class Settings;

class McpViewModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool running READ IsRunning NOTIFY changed)
  Q_PROPERTY(bool busy READ IsBusy NOTIFY changed)
  Q_PROPERTY(QString url READ Url NOTIFY changed)
  Q_PROPERTY(QString error READ Error NOTIFY changed)
  Q_PROPERTY(int port READ Port NOTIFY changed)
public:
  McpViewModel(WorkerLaunchConfiguration configuration, Settings* settings, QObject* parent = nullptr);
  bool IsRunning() const { return running_; }
  bool IsBusy() const { return busy_; }
  QString Url() const { return url_; }
  QString Error() const { return error_; }
  int Port() const;
  Q_INVOKABLE void SetPort(int port);
  Q_INVOKABLE void setPort(int port) { SetPort(port); }
  Q_INVOKABLE void SetEnabled(bool enabled);
  Q_INVOKABLE void setEnabled(bool enabled) { SetEnabled(enabled); }
  Q_INVOKABLE void Refresh();
signals:
  void changed();
private:
  WorkerControlClient worker_;
  Settings* settings_;
  QTimer poll_;
  bool running_ = false;
  bool busy_ = false;
  QString url_;
  QString error_;
};
