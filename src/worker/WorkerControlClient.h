#pragma once

#include "yh/workercontrol_export.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>

struct WorkerLaunchConfiguration {
  QString control_name;
  QString executable;
  QStringList arguments;
  QString stop_command = "stop";
};

// Application-independent local IPC and detached process lifecycle. A successful
// Stop waits for the process to exit; a timeout never claims it has stopped.
class WORKERCONTROL_EXPORT WorkerControlClient final : public QObject {
  Q_OBJECT
public:
  enum class Command { Start, Stop, Status, Send };
  static constexpr int DefaultTimeoutMs = 30'000;
  explicit WorkerControlClient(WorkerLaunchConfiguration configuration, QObject* parent = nullptr);
  ~WorkerControlClient() override;
  void Request(Command command, const QJsonObject& message = {},
               const QStringList& launch_arguments = {}, int timeout_ms = DefaultTimeoutMs);
  bool IsBusy() const;

signals:
  void finished(const QJsonObject& result);

private:
  struct Operation;
  void Probe(Operation* operation);
  void Unavailable(Operation* operation);
  void Receive(Operation* operation);
  void Finish(Operation* operation, QJsonObject result);
  static void Retire(Operation* operation);
  qint64 KnownProcess(Operation* operation, bool* owned = nullptr);
  WorkerLaunchConfiguration configuration_;
  Operation* operation_ = nullptr;
  qint64 launched_pid_ = 0;
};
