#include "RestControlService.h"
#include <QJsonDocument>
#include <QProcess>

namespace {
constexpr int RetryIntervalMs = 100;
constexpr qsizetype MaxResponseBytes = 64 * 1024;

QJsonObject Error(const QString& code, const QString& message) {
  return {{"ok", false}, {"error", QJsonObject{{"code", code}, {"message", message}}}};
}
} // namespace

RestControlService::RestControlService(
  Settings* settings, RestLaunchConfiguration configuration, QObject* parent)
  : QObject(parent), settings_(settings), configuration_(std::move(configuration)) {
  timeout_.setSingleShot(true);
  retry_.setSingleShot(true);
  connect(&retry_, &QTimer::timeout, this, &RestControlService::Probe);
  connect(&timeout_, &QTimer::timeout, this, [this] {
    Finish(Error("timeout", tr("Время управления REST API истекло.")));
  });
  connect(&socket_, &QLocalSocket::connected, this, [this] {
    const QString command = command_ == Command::Disable ? "disable"
                                                         : "status";
    socket_.write(
      QJsonDocument(QJsonObject{{"command", command}}).toJson(QJsonDocument::Compact) + '\n');
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
  socket_.abort();
  buffer_.clear();
  socket_.connectToServer(configuration_.control_name);
}

QJsonObject RestControlService::Status() const {
  return {{"ok", true}, {"enabled", settings_->GetRestEnabled()}, {"running", false},
    {"fixture", configuration_.fixture}, {"port", settings_->GetRestPort()},
    {"url", QString("http://127.0.0.1:%1/v1").arg(settings_->GetRestPort())}};
}

void RestControlService::Unavailable(QLocalSocket::LocalSocketError error) {
  if (!busy_) {
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
      if (!worker.startDetached()) {
        Finish(Error("startup_error", tr("Не удалось запустить сервер REST API.")));
        return;
      }
    }
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
  if (command_ == Command::Enable && result["ok"].toBool()) {
    if (!result["running"].toBool()) {
      retry_.start(RetryIntervalMs);
      return;
    }
    const auto port = static_cast<quint16>(result["port"].toInt());
    if (requested_port_ && port != *requested_port_) {
      Finish(Error(
        "port_conflict", tr("REST API уже работает на другом порту. Сначала отключите его.")));
      return;
    }
    settings_->SetRestPort(port);
    settings_->SetRestEnabled(true);
  }
  Finish(result);
}

void RestControlService::Finish(QJsonObject result) {
  if (!busy_) {
    return;
  }
  busy_ = false;
  timeout_.stop();
  retry_.stop();
  socket_.abort();
  emit finished(result);
}
