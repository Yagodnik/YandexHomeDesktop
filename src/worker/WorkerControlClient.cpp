#include "WorkerControlClient.h"
#include "WorkerControlPrivate.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QLockFile>
#include <QPointer>
#include <QProcess>
#include <QSaveFile>
#include <QTimer>
#include <cmath>
#include <memory>

using namespace WorkerControlPrivate;

struct WorkerControlClient::Operation : QObject {
  explicit Operation(QObject* parent, const QString& name)
      : QObject(parent), paths(ControlPaths(name)), transition(paths.transition) {
    transition.setStaleLockTime(0);
    deadline.setSingleShot(true);
    retry.setSingleShot(true);
  }
  Paths paths;
  QLockFile transition;
  QTimer deadline;
  QTimer retry;
  QLocalSocket* socket = nullptr;
  QByteArray buffer;
  QJsonObject message;
  QJsonObject stopped;
  QStringList launch_arguments;
  Command command = Command::Status;
  qint64 pid = 0;
  bool launched = false;
};

WorkerControlClient::WorkerControlClient(WorkerLaunchConfiguration configuration, QObject* parent)
    : QObject(parent), configuration_(std::move(configuration)) {}

WorkerControlClient::~WorkerControlClient() {
  Retire(operation_);
}

bool WorkerControlClient::IsBusy() const {
  return operation_ && operation_->command != Command::Status;
}

void WorkerControlClient::Request(Command command, const QJsonObject& message,
                                  const QStringList& launch_arguments, int timeout_ms) {
  if (operation_ && command != Command::Stop &&
      !(operation_->command == Command::Status && command == Command::Start)) {
    return;
  }
  // Each request owns its sockets/timers. Queued events from a replaced request
  // cannot be interpreted as responses to its replacement.
  Retire(operation_);
  auto* operation = new Operation(this, configuration_.control_name);
  operation_ = operation;
  operation->pid = launched_pid_;
  operation->command = command;
  operation->launch_arguments = launch_arguments;
  operation->message =
      command == Command::Send
          ? message
          : QJsonObject{
                {"command", command == Command::Stop ? configuration_.stop_command : "status"}};
  connect(&operation->deadline, &QTimer::timeout, operation,
          [this, operation] { Finish(operation, Error("timeout")); });
  connect(&operation->retry, &QTimer::timeout, operation, [this, operation] { Probe(operation); });
  operation->deadline.start(qMax(1, timeout_ms));
  if (configuration_.control_name.isEmpty() || !PrepareDirectory(operation->paths)) {
    Finish(operation, Error("control_error"));
    return;
  }
  Probe(operation);
}

qint64 WorkerControlClient::KnownProcess(Operation* operation, bool* owned) {
  if (owned) {
    *owned = false;
  }
  QLockFile ownership(operation->paths.ownership);
  // Observation must never acquire the worker's lock: even a momentary probe
  // could make a concurrently starting foreground worker reject ownership.
  qint64 owner_pid = 0;
  QString host, app;
  if (ownership.getLockInfo(&owner_pid, &host, &app)) {
    if (ProcessIsRunning(owner_pid)) {
      if (owned)
        *owned = true;
      return owner_pid;
    }
  } else if (owned && QFileInfo::exists(operation->paths.ownership)) {
    // A worker may still be writing its ownership record.
    *owned = true;
  }
  if (ProcessIsRunning(operation->pid)) {
    return operation->pid;
  }
  const auto pid = LaunchPid(operation->paths);
  return ProcessIsRunning(pid) ? pid : 0;
}

void WorkerControlClient::Probe(Operation* operation) {
  if (operation != operation_) {
    return;
  }
  if (operation->command == Command::Start || operation->command == Command::Stop) {
    if (!operation->transition.isLocked() && !operation->transition.tryLock()) {
      if (operation->transition.error() != QLockFile::LockFailedError) {
        Finish(operation, Error("control_error"));
      } else {
        operation->retry.start(RetryIntervalMs);
      }
      return;
    }
  }
  if (!operation->stopped.isEmpty()) {
    if (ProcessIsRunning(operation->pid)) {
      operation->retry.start(RetryIntervalMs);
      return;
    }
    ClearLaunch(operation->paths, operation->pid);
    Finish(operation, operation->stopped);
    return;
  }
  if (operation->socket) {
    delete operation->socket;
  }
  operation->buffer.clear();
  auto* socket = new QLocalSocket(operation);
  operation->socket = socket;
  socket->setReadBufferSize(MaxResponseBytes + 1);
  connect(socket, &QLocalSocket::connected, operation, [operation] {
    operation->socket->write(QJsonDocument(operation->message).toJson(QJsonDocument::Compact) +
                             '\n');
  });
  connect(socket, &QLocalSocket::readyRead, operation, [this, operation] { Receive(operation); });
  connect(socket, &QLocalSocket::disconnected, operation, [this, operation] {
    if (operation != operation_) {
      return;
    }
    // Windows named pipes can disconnect without emitting errorOccurred.
    // Consume any final frame before treating an unanswered request as lost.
    QPointer<WorkerControlClient> alive(this);
    if (operation->socket->bytesAvailable() > 0) {
      Receive(operation);
    }
    if (!alive || operation != operation_ || operation->retry.isActive()) {
      return;
    }
    if (operation->command == Command::Send) {
      Finish(operation, Error("control_error"));
    } else {
      Unavailable(operation);
    }
  });
  connect(socket, &QLocalSocket::errorOccurred, operation,
          [this, operation](QLocalSocket::LocalSocketError error) {
            if (operation != operation_) {
              return;
            }
            if (error == QLocalSocket::ServerNotFoundError ||
                error == QLocalSocket::ConnectionRefusedError ||
                (error == QLocalSocket::PeerClosedError && operation->command != Command::Send)) {
              Unavailable(operation);
            } else {
              Finish(operation, Error("control_error"));
            }
          });
  socket->connectToServer(configuration_.control_name);
}

void WorkerControlClient::Unavailable(Operation* operation) {
  bool owned = false;
  const auto pid = KnownProcess(operation, &owned);
  if (operation->command == Command::Start) {
    if (pid > 0 || owned) {
      operation->retry.start(RetryIntervalMs);
      return;
    }
    if (operation->launched) {
      ClearLaunch(operation->paths, operation->pid);
      Finish(operation, Error("startup_error"));
      return;
    }
    // Reserve a writable atomic receipt before launching. The receipt lets a
    // different controller find a child before its control socket exists, even
    // after this controller times out or is destroyed.
    QSaveFile receipt(operation->paths.launch);
    receipt.setDirectWriteFallback(false);
    if (!receipt.open(QIODevice::WriteOnly)) {
      Finish(operation, Error("control_error"));
      return;
    }
    receipt.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QProcess worker;
    worker.setProgram(configuration_.executable);
    worker.setArguments(configuration_.arguments + operation->launch_arguments);
    worker.setStandardOutputFile(QProcess::nullDevice());
    worker.setStandardErrorFile(QProcess::nullDevice());
    if (!worker.startDetached(&operation->pid)) {
      Finish(operation, Error("startup_error"));
      return;
    }
    operation->launched = true;
    launched_pid_ = operation->pid;
    const auto bytes = QByteArray::number(operation->pid);
    if (receipt.write(bytes) != bytes.size() || !receipt.commit()) {
      // Retain the PID so this controller can still issue Stop; never claim
      // that a launched process has stopped when its receipt cannot be saved.
      Finish(operation, Error("control_error"));
      return;
    }
    operation->retry.start(RetryIntervalMs);
  } else if (operation->command == Command::Stop && (pid > 0 || owned)) {
    operation->pid = pid;
    operation->retry.start(RetryIntervalMs);
  } else if (operation->command == Command::Send) {
    Finish(operation, Error("control_error"));
  } else {
    const auto recorded = LaunchPid(operation->paths);
    if (operation->command == Command::Stop) {
      ClearLaunch(operation->paths, recorded);
    }
    Finish(operation, {{"ok", true}, {"running", false}, {"starting", pid > 0 || owned}});
  }
}

void WorkerControlClient::Receive(Operation* operation) {
  if (operation != operation_) {
    return;
  }
  operation->buffer += operation->socket->readAll();
  if (operation->buffer.size() > MaxResponseBytes) {
    Finish(operation, Error("invalid_response"));
    return;
  }
  const auto end = operation->buffer.indexOf('\n');
  if (end < 0) {
    return;
  }
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(operation->buffer.left(end), &error);
  auto result = document.object();
  const auto pid = result.value("pid");
  if (error.error != QJsonParseError::NoError || !document.isObject() || !result["ok"].isBool() ||
      (result["ok"].toBool() && operation->command != Command::Send &&
       !result["running"].isBool()) ||
      (!pid.isUndefined() &&
       (!pid.isDouble() || pid.toInteger() <= 0 || std::floor(pid.toDouble()) != pid.toDouble()))) {
    Finish(operation, Error("invalid_response"));
    return;
  }
  if (pid.toInteger() > 0) {
    operation->pid = pid.toInteger();
    launched_pid_ = operation->pid;
  }
  if (operation->command == Command::Start && result["ok"].toBool() &&
      !result["running"].toBool()) {
    operation->retry.start(RetryIntervalMs);
    return;
  }
  if (operation->command == Command::Stop && result["ok"].toBool()) {
    if (result["running"].toBool()) {
      Finish(operation, Error("invalid_response"));
      return;
    }
    operation->stopped = result;
    operation->socket->disconnect(operation);
    operation->socket->abort();
    Probe(operation);
    return;
  }
  Finish(operation, result);
}

void WorkerControlClient::Finish(Operation* operation, QJsonObject result) {
  if (operation != operation_) {
    return;
  }
  operation_ = nullptr;
  Retire(operation);
  emit finished(result);
}

void WorkerControlClient::Retire(Operation* operation) {
  if (!operation)
    return;
  operation->deadline.stop();
  operation->retry.stop();
  operation->deadline.disconnect(operation);
  operation->retry.disconnect(operation);
  if (operation->socket) {
    operation->socket->disconnect(operation);
    operation->socket->abort();
  }
  operation->transition.unlock();
  // Completion handlers may destroy the client inside a socket's readyRead
  // signal. Defer destruction of that socket until its own event has returned.
  operation->setParent(nullptr);
  operation->deleteLater();
}
