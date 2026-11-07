#include "FixtureInteractiveAuthorizationService.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

FixtureInteractiveAuthorizationService::FixtureInteractiveAuthorizationService(QObject* parent)
    : IAuthorizationService(parent) {
  connect(&server_, &QTcpServer::newConnection, this, [this] { AcceptConnections(); });
}
QString FixtureInteractiveAuthorizationService::AuthorizationUrl() const {
  return pending_ ? QString("http://127.0.0.1:%1/").arg(server_.serverPort()) : QString{};
}
void FixtureInteractiveAuthorizationService::AttemptLocalAuthorization() {
  if (logout_requested_ || pending_)
    return;
  const auto generation = ++generation_;
  QTimer::singleShot(0, this, [this, generation] {
    if (generation != generation_)
      return;
    if (authorized_)
      emit authorized();
    else
      emit unauthorized();
  });
}
void FixtureInteractiveAuthorizationService::AttemptAuthorization() {
  if (logout_requested_ || pending_)
    return;
  ++generation_;
  if (!server_.listen(QHostAddress::LocalHost, 0)) {
    emit authorizationFailed();
    return;
  }
  pending_ = true;
  emit authorizationStateChanged();
}
void FixtureInteractiveAuthorizationService::Stop() {
  server_.close();
  const auto sockets = server_.findChildren<QTcpSocket*>();
  for (auto* socket : sockets) {
    socket->disconnect(this);
    socket->abort();
    socket->deleteLater();
  }
  if (pending_) {
    pending_ = false;
    emit authorizationStateChanged();
  }
}
void FixtureInteractiveAuthorizationService::CancelAuthorization() {
  if (!pending_)
    return;
  ++generation_;
  Stop();
}
void FixtureInteractiveAuthorizationService::Logout() {
  if (logout_requested_)
    return;
  ++generation_;
  logout_requested_ = true;
  authorized_ = false;
  Stop();
  emit logout();
  QTimer::singleShot(0, this, [this] {
    logout_requested_ = false;
    emit logoutFinished();
  });
}
void FixtureInteractiveAuthorizationService::AcceptConnections() {
  while (auto* socket = server_.nextPendingConnection()) {
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
      auto data = socket->property("request").toByteArray() + socket->readAll();
      if (data.size() > 8192) {
        socket->disconnectFromHost();
        return;
      }
      socket->setProperty("request", data);
      if (data.contains("\r\n\r\n")) {
        socket->disconnect(this);
        Reply(socket, data);
      }
    });
  }
}
void FixtureInteractiveAuthorizationService::Reply(QTcpSocket* socket, const QByteArray& request) {
  const auto firstLine = request.left(request.indexOf("\r\n")).split(' ');
  const QUrl path = firstLine.size() == 3 && firstLine[0] == "GET"
                        ? QUrl::fromEncoded(firstLine[1], QUrl::StrictMode)
                        : QUrl();
  const bool confirmed = pending_ && path.path() == "/confirm" &&
                         QUrlQuery(path).queryItemValue("code") == AuthorizationCode();
  const auto trText = [](const char* source) {
    return QCoreApplication::translate("FixtureAuthPage", source).toHtmlEscaped();
  };
  QString body;
  if (confirmed) {
    body = "<p>" + trText("Готово. Вернитесь в приложение.") + "</p>";
  } else {
    body = "<h1>" + trText("Демонстрационная авторизация") + "</h1><p>" +
           trText("Введите код из приложения. Данные никуда не отправляются.") +
           "</p><form action=\"/confirm\"><input name=\"code\" autofocus required> <button>" +
           trText("Подтвердить") + "</button></form>";
  }
  const auto html = ("<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" "
                     "content=\"width=device-width, initial-scale=1\"></head><body>" +
                     body + "</body></html>")
                        .toUtf8();
  socket->write("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-Control: "
                "no-store\r\nConnection: close\r\nContent-Length: " +
                QByteArray::number(html.size()) + "\r\n\r\n" + html);
  socket->disconnectFromHost();
  if (confirmed) {
    // Let the response flush before closing the listener and other sockets.
    socket->setParent(this);
    ++generation_;
    authorized_ = true;
    Stop();
    emit authorized();
  }
}
