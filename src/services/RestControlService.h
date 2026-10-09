#pragma once

#include "yh/apprestcontrol_export.h"

#include <QJsonObject>
#include <QLocalSocket>
#include <QTimer>
#include <optional>
#include "utils/Settings.h"

struct RestLaunchConfiguration {
  QString control_name;
  QString executable;
  QStringList arguments;
  bool fixture = false;
};

// GUI and CLI share this asynchronous lifecycle controller. It never reads OAuth tokens.
class APPRESTCONTROL_EXPORT RestControlService final : public QObject {
  Q_OBJECT
public:
  enum class Command { Enable, Disable, Status };
  static constexpr int DefaultTimeoutMs = 30'000;
  RestControlService(
    Settings* settings, RestLaunchConfiguration configuration, QObject* parent = nullptr);
  void Request(
    Command command, std::optional<quint16> port = {}, int timeout_ms = DefaultTimeoutMs);
  bool IsBusy() const;
signals:
  void finished(const QJsonObject& result);

private:
  void Probe();
  void Unavailable(QLocalSocket::LocalSocketError error);
  void Receive();
  void Finish(QJsonObject result);
  QJsonObject Status() const;
  Settings* settings_;
  RestLaunchConfiguration configuration_;
  QLocalSocket socket_;
  QTimer timeout_;
  QTimer retry_;
  QByteArray buffer_;
  Command command_ = Command::Status;
  std::optional<quint16> requested_port_;
  bool busy_ = false;
  bool launched_ = false;
};
