#include "WorkerControlServer.h"
#include "WorkerControlPrivate.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QPointer>
#include <QSaveFile>
#include <QTimer>

using namespace WorkerControlPrivate;

WorkerControlServer::WorkerControlServer(QObject* parent) : QObject(parent) {
  connect(&server_, &QLocalServer::newConnection, this, &WorkerControlServer::AcceptConnections);
}

WorkerControlServer::~WorkerControlServer() {
  Close();
  // Keep receipts until process exit. Clients also use them after the socket
  // and ownership lock have been released by application cleanup.
}

bool WorkerControlServer::Acquire(const QString& control_name) {
  if (ownership_) {
    return name_ == control_name;
  }
  if (control_name.isEmpty()) {
    error_ = "control_error";
    return false;
  }
  const auto paths = ControlPaths(control_name);
  if (!PrepareDirectory(paths)) {
    error_ = "control_error";
    return false;
  }
  auto lock = std::make_unique<QLockFile>(paths.ownership);
  lock->setStaleLockTime(0);
  if (!lock->tryLock()) {
    error_ = lock->error() == QLockFile::LockFailedError ? "already_running" : "control_error";
    return false;
  }
  // Foreground workers need the same cleanup receipt as detached workers: the
  // socket and ownership object may be destroyed before the process exits.
  QSaveFile receipt(paths.launch);
  receipt.setDirectWriteFallback(false);
  const auto pid = QByteArray::number(QCoreApplication::applicationPid());
  if (!receipt.open(QIODevice::WriteOnly) ||
      !receipt.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner) ||
      receipt.write(pid) != pid.size() || !receipt.commit()) {
    error_ = "control_error";
    return false;
  }
  name_ = control_name;
  ownership_ = std::move(lock);
  error_.clear();
  return true;
}

bool WorkerControlServer::Listen() {
  if (!ownership_) {
    error_ = "control_error";
    return false;
  }
  if (server_.isListening()) {
    return true;
  }
  server_.setSocketOptions(QLocalServer::UserAccessOption);
  // Only the holder of the process ownership lock may unlink a stale socket.
  QLocalServer::removeServer(name_);
  if (!server_.listen(name_)) {
    error_ = "control_error";
    return false;
  }
  error_.clear();
  return true;
}

bool WorkerControlServer::Start(const QString& control_name) {
  return Acquire(control_name) && Listen();
}

void WorkerControlServer::Close() {
  server_.close();
  const auto sockets = findChildren<QLocalSocket*>(QString{}, Qt::FindDirectChildrenOnly);
  for (auto* socket : sockets) {
    socket->disconnect(this);
    socket->abort();
    socket->deleteLater();
  }
}

void WorkerControlServer::SetHandler(Handler handler) {
  handler_ = std::move(handler);
}

QString WorkerControlServer::ErrorCode() const {
  return error_;
}

void WorkerControlServer::AcceptConnections() {
  while (auto* socket = server_.nextPendingConnection()) {
    socket->setParent(this);
    socket->setReadBufferSize(MaxRequestBytes + 1);
    auto buffer = std::make_shared<QByteArray>();
    auto handled = std::make_shared<bool>(false);
    auto* deadline = new QTimer(socket);
    deadline->setSingleShot(true);
    connect(deadline, &QTimer::timeout, socket, &QLocalSocket::abort);
    deadline->start(5'000);
    connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QLocalSocket::readyRead, this, [this, socket, buffer, handled, deadline] {
      if (*handled) {
        return;
      }
      *buffer += socket->readAll();
      if (buffer->size() > MaxRequestBytes) {
        socket->abort();
        return;
      }
      const auto end = buffer->indexOf('\n');
      if (end < 0) {
        return;
      }
      *handled = true;
      deadline->stop();
      QJsonParseError parse_error;
      const auto document = QJsonDocument::fromJson(buffer->left(end), &parse_error);
      const auto request = document.object();
      QJsonObject response;
      const auto handler = handler_;
      QPointer<WorkerControlServer> alive(this);
      QPointer<QLocalSocket> peer(socket);
      // A command may destroy the server from inside this socket's read event.
      // Keep the emitting socket alive until that event has unwound.
      socket->setParent(nullptr);
      if (parse_error.error != QJsonParseError::NoError || !document.isObject() ||
          !request["command"].isString() || request["command"].toString().isEmpty()) {
        response = Error("invalid_request");
      } else if (handler) {
        // A handler may close or destroy its server while processing a command.
        response = handler(request);
      } else {
        response = Error("unsupported_command");
      }
      if (!alive || !peer || peer->state() != QLocalSocket::ConnectedState) {
        if (peer) {
          peer->abort();
          peer->deleteLater();
        }
        return;
      }
      peer->setParent(this);
      response["pid"] = QCoreApplication::applicationPid();
      auto bytes = QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n';
      if (bytes.size() > MaxResponseBytes) {
        bytes = QJsonDocument(Error("response_too_large")).toJson(QJsonDocument::Compact) + '\n';
      }
      peer->write(bytes);
      peer->disconnectFromServer();
    });
  }
}
