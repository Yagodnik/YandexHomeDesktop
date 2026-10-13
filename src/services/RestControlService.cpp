#include "RestControlService.h"
#include <QJsonDocument>
#include <QProcess>
#include <QFile>
#include <QUrl>
#include <limits>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <cerrno>
#include <csignal>
#ifdef Q_OS_MACOS
#include <sys/proc.h>
#include <sys/sysctl.h>
#endif
#endif

namespace {
constexpr int RetryIntervalMs = 100;
constexpr qsizetype MaxResponseBytes = 64 * 1024;

bool ProcessIsRunning(qint64 pid) {
  if (pid <= 0) {
    return false;
  }
#ifdef Q_OS_WIN
  if (pid > std::numeric_limits<DWORD>::max()) {
    return false;
  }
  const auto process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
  if (!process) {
    return GetLastError() != ERROR_INVALID_PARAMETER;
  }
  const auto state = WaitForSingleObject(process, 0);
  CloseHandle(process);
  return state != WAIT_OBJECT_0;
#else
  if (pid > std::numeric_limits<pid_t>::max()) {
    return false;
  }
  if (::kill(static_cast<pid_t>(pid), 0) != 0) {
    return errno != ESRCH;
  }
  // A foreground worker can remain a zombie until its supervisor reaps it.
  // Waiting for that supervisor here would deadlock clients that reap only
  // after disable returns; the worker has already completed cleanup.
#ifdef Q_OS_MACOS
  int query[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(pid)};
  kinfo_proc info{};
  size_t size = sizeof(info);
  if (sysctl(query, 4, &info, &size, nullptr, 0) == 0) {
    return size != 0 && info.kp_proc.p_stat != SZOMB;
  }
#elif defined(Q_OS_LINUX)
  QFile stat(QString("/proc/%1/stat").arg(pid));
  if (stat.open(QIODevice::ReadOnly)) {
    const auto bytes = stat.readAll();
    const auto end = bytes.lastIndexOf(')');
    if (end >= 0 && end + 2 < bytes.size()) {
      return bytes[end + 2] != 'Z' && bytes[end + 2] != 'X';
    }
  }
#endif
  return true;
#endif
}

QJsonObject Error(const QString& code, const QString& message) {
  return {{"ok", false}, {"error", QJsonObject{{"code", code}, {"message", message}}}};
}
} // namespace

RestControlService::RestControlService(Settings* settings, RestLaunchConfiguration configuration,
                                       QObject* parent)
    : QObject(parent), settings_(settings), configuration_(std::move(configuration)) {
  timeout_.setSingleShot(true);
  retry_.setSingleShot(true);
  connect(&retry_, &QTimer::timeout, this, &RestControlService::Probe);
  connect(&timeout_, &QTimer::timeout, this,
          [this] { Finish(Error("timeout", tr("Время управления REST API истекло."))); });
  connect(&socket_, &QLocalSocket::connected, this, [this] {
    const QString command = command_ == Command::Disable ? "disable" : "status";
    socket_.write(QJsonDocument(QJsonObject{{"command", command}}).toJson(QJsonDocument::Compact) +
                  '\n');
  });
  connect(&socket_, &QLocalSocket::readyRead, this, &RestControlService::Receive);
  connect(&socket_, &QLocalSocket::errorOccurred, this, &RestControlService::Unavailable);
}

bool RestControlService::IsBusy() const {
  return busy_ && command_ != Command::Status;
}

void RestControlService::Request(Command command, std::optional<quint16> port, int timeout_ms) {
  // Disable supersedes a pending start, for example on logout.
  if (busy_ && command != Command::Disable &&
      !(command_ == Command::Status && command == Command::Enable)) {
    return;
  }
  busy_ = false;
  socket_.abort();
  retry_.stop();
  shutdown_result_ = {};
  command_ = command;
  requested_port_ = port;
  launched_ = false;
  busy_ = true;
  if (command == Command::Disable) {
    settings_->SetRestEnabled(false);
  }
  timeout_.start(timeout_ms);
  Probe();
}

void RestControlService::Probe() {
  if (!busy_) {
    return;
  }
  if (!shutdown_result_.isEmpty()) {
    WaitForExit();
    return;
  }
  socket_.abort();
  buffer_.clear();
  socket_.connectToServer(configuration_.control_name);
}

QJsonObject RestControlService::Status() const {
  const auto host = configuration_.host.isEmpty() ? QString("127.0.0.1") : configuration_.host;
  QUrl url;
  url.setScheme("http");
  url.setHost(host);
  url.setPort(settings_->GetRestPort());
  url.setPath("/v1");
  return {{"ok", true},
          {"enabled", settings_->GetRestEnabled()},
          {"running", false},
          {"fixture", configuration_.fixture},
          {"port", settings_->GetRestPort()},
          {"host", host},
          {"url", url.toString()}};
}

void RestControlService::Unavailable(QLocalSocket::LocalSocketError error) {
  if (!busy_ || !shutdown_result_.isEmpty()) {
    return;
  }
  if (error != QLocalSocket::ServerNotFoundError && error != QLocalSocket::ConnectionRefusedError &&
      error != QLocalSocket::PeerClosedError) {
    Finish(Error("control_error", tr("Не удалось подключиться к серверу REST API.")));
    return;
  }
  if (command_ == Command::Enable) {
    if (!launched_) {
      launched_ = true;
      auto arguments = configuration_.arguments;
      arguments << "--serve-rest" << "--rest-port"
                << QString::number(requested_port_.value_or(settings_->GetRestPort()))
                << "--timeout" << QString::number(timeout_.remainingTime());
      QProcess worker;
      worker.setProgram(configuration_.executable);
      worker.setArguments(arguments);
      worker.setStandardOutputFile(QProcess::nullDevice());
      worker.setStandardErrorFile(QProcess::nullDevice());
      if (!worker.startDetached(&worker_pid_)) {
        Finish(Error("startup_error", tr("Не удалось запустить сервер REST API.")));
        return;
      }
    }
    retry_.start(RetryIntervalMs);

  } else if (command_ == Command::Disable && ProcessIsRunning(worker_pid_)) {
    // A superseded start may not have opened its control socket yet. Keep
    // trying until it accepts disable or exits, within the command timeout.
    retry_.start(RetryIntervalMs);
  } else {
    Finish(Status());
  }
}

void RestControlService::Receive() {
  if (!busy_) {
    return;
  }
  buffer_ += socket_.readAll();
  if (buffer_.size() > MaxResponseBytes) {
    Finish(Error("invalid_response", tr("Некорректный ответ сервера REST API.")));
    return;
  }
  const auto end = buffer_.indexOf('\n');
  if (end < 0) {
    return;
  }
  QJsonParseError error;
  const auto json = QJsonDocument::fromJson(buffer_.left(end), &error);
  if (error.error != QJsonParseError::NoError || !json.isObject() ||
      !json.object().contains("ok")) {
    Finish(Error("invalid_response", tr("Некорректный ответ сервера REST API.")));
    return;
  }
  auto result = json.object();
  if (result["ok"].toBool() && result["pid"].toInteger() > 0) {
    worker_pid_ = result["pid"].toInteger();
  }
  if (command_ == Command::Enable && result["ok"].toBool()) {
    if (!result["running"].toBool()) {
      retry_.start(RetryIntervalMs);
      return;
    }
    const auto port = static_cast<quint16>(result["port"].toInt());
    if (requested_port_ && port != *requested_port_) {
      Finish(Error("port_conflict",
                   tr("REST API уже работает на другом порту. Сначала отключите его.")));
      return;
    }
    if (!configuration_.host.isEmpty() && result["host"].toString() != configuration_.host) {
      Finish(Error("host_conflict",
                   tr("REST API is already running on another host. Disable it first.")));
      return;
    }
    settings_->SetRestPort(port);
    settings_->SetRestEnabled(true);
  }
  if (command_ == Command::Disable && result["ok"].toBool() && worker_pid_ > 0) {
    // The reply acknowledges listener shutdown. The detached worker still
    // owns its process and lock until cleanup finishes.
    shutdown_result_ = result;
    socket_.abort();
    WaitForExit();
    return;
  }
  Finish(result);
}

void RestControlService::WaitForExit() {
  if (ProcessIsRunning(worker_pid_)) {
    retry_.start(RetryIntervalMs);
    return;
  }
  worker_pid_ = 0;
  Finish(shutdown_result_);
}

void RestControlService::Finish(QJsonObject result) {
  if (!busy_) {
    return;
  }
  busy_ = false;
  timeout_.stop();
  retry_.stop();
  socket_.abort();
  shutdown_result_ = {};
  emit finished(result);
}
