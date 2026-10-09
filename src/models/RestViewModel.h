#pragma once
#include "services/RestControlService.h"

class RestViewModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool enabled READ IsEnabled NOTIFY changed)
  Q_PROPERTY(bool running READ IsRunning NOTIFY changed)
  Q_PROPERTY(bool busy READ IsBusy NOTIFY changed)
  Q_PROPERTY(QString url READ GetUrl NOTIFY changed)
  Q_PROPERTY(QString error READ GetError NOTIFY changed)
public:
  explicit RestViewModel(RestControlService* service, QObject* parent = nullptr);
  bool IsEnabled() const;
  bool IsRunning() const;
  bool IsBusy() const;
  QString GetUrl() const;
  QString GetError() const;
  Q_INVOKABLE void SetEnabled(bool enabled);
  Q_INVOKABLE void Refresh();
signals:
  void changed();

private:
  RestControlService* service_;
  QTimer poll_;
  bool enabled_ = false;
  bool running_ = false;
  QString url_;
  QString error_;
};
