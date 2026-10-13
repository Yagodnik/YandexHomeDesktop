#include "RestServer.h"

#include <QTcpSocket>

#include "RestReply.h"
#include "RestRouter.h"

RestServer::RestServer(const RestRouter& routes, QObject* parent, int timeout_ms)
    : QObject(parent), routes_(routes), timeout_ms_(timeout_ms) {
  http_.setMissingHandler(this,
                          [this](const QHttpServerRequest& request,
                                 QHttpServerResponder& responder) { Handle(request, responder); });
}

RestServer::~RestServer() {
  Stop();
}

bool RestServer::Start(quint16 port) {
  return Start(port, QHostAddress::LocalHost);
}

bool RestServer::Start(quint16 port, const QHostAddress& host) {
  if (tcp_.isListening()) {
    return port == Port() && host == tcp_.serverAddress();
  }
  if (host.isNull() || !tcp_.listen(host, port)) {
    return false;
  }
  if (!http_.servers().contains(&tcp_) && !http_.bind(&tcp_)) {
    tcp_.close();
    return false;
  }
  return true;
}

void RestServer::Stop() {
  tcp_.close();

  // Cancel only request contexts before closing their streams.
  const auto requests = requests_.children();
  for (auto* request : requests) {
    delete request;
  }
  for (auto* socket : tcp_.findChildren<QTcpSocket*>()) {
    socket->abort();
  }
}

quint16 RestServer::Port() const {
  return tcp_.serverPort();
}

QString RestServer::Error() const {
  return tcp_.errorString();
}

void RestServer::Handle(const QHttpServerRequest& request, QHttpServerResponder& responder) {
  auto* reply = new RestReply(std::move(responder), &requests_, timeout_ms_);
  if (!tcp_.isListening()) {
    reply->Fail(RestReply::StatusCode::ServiceUnavailable, "disabled", tr("REST API отключён."));
    return;
  }
  if (!request.headers().value("Origin").isEmpty()) {
    reply->Fail(RestReply::StatusCode::Forbidden, "origin_forbidden",
                tr("Запросы из браузера не разрешены."));
    return;
  }
  // Match the authority to the actual connection's destination, including
  // IPv6 and wildcard listeners. Localhost is an alias for loopback only.
  const auto host = request.headers().value("Host").toByteArray().toLower();
  const auto authority = QUrl::fromEncoded("http://" + host, QUrl::StrictMode);
  const QHostAddress address(authority.host());
  const bool matches_address =
      address.isEqual(request.localAddress(), QHostAddress::ConvertV4MappedToIPv4) ||
      (authority.host() == "localhost" && request.localAddress().isLoopback());
  if (!authority.isValid() || !authority.userInfo().isEmpty() || !authority.path().isEmpty() ||
      authority.hasQuery() || authority.hasFragment() || authority.port() != Port() ||
      !matches_address) {
    reply->Fail(RestReply::StatusCode::Forbidden, "host_forbidden", tr("Invalid REST API host."));
    return;
  }
  if (request.method() == QHttpServerRequest::Method::Post &&
      request.headers()
              .value("Content-Type")
              .toByteArray()
              .split(';')
              .first()
              .trimmed()
              .toLower() != "application/json") {
    reply->Fail(RestReply::StatusCode::UnsupportedMediaType, "content_type",
                tr("Content-Type must be application/json."));
    return;
  }
  routes_.Handle(request, reply);
}
