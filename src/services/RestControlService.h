#pragma once

#include "yh/apprestcontrol_export.h"

#include <QJsonObject>
#include "worker/WorkerControlClient.h"
#include <optional>
#include "utils/Settings.h"

struct RestLaunchConfiguration {
  QString control_name;
  QString executable;
  QStringList arguments;
  bool fixture = false;
  QString host;
};

// GUI and CLI share this asynchronous lifecycle controller. It never reads OAuth tokens.
class APPRESTCONTROL_EXPORT RestControlService final : public QObject {
  Q_OBJECT
public:
  enum class Command { Enable, Disable, Status };
  static constexpr int DefaultTimeoutMs = 30'000;
  RestControlService(Settings* settings, RestLaunchConfiguration configuration,
                     QObject* parent = nullptr);
  void Request(Command command, std::optional<quint16> port = {},
               int timeout_ms = DefaultTimeoutMs);
  bool IsBusy() const;
  quint16 Port() const { return settings_->GetRestPort(); }
  void SetPort(quint16 port) { settings_->SetRestPort(port); }
signals:
  void finished(const QJsonObject& result);

private:
  QJsonObject Status() const;
  Settings* settings_;
  RestLaunchConfiguration configuration_;
  WorkerControlClient worker_;
  Command command_ = Command::Status;
  std::optional<quint16> requested_port_;
  bool busy_ = false;
};
