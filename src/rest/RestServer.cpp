#include "RestServer.h"

#include <QCryptographicHash>
#include <QTcpSocket>

#include "RestReply.h"
#include "RestRouter.h"

namespace {
QByteArray TokenDigest(QByteArrayView token) {
  return QCryptographicHash::hash(token, QCryptographicHash::Sha256);
}

bool MatchesBearer(const QHttpHeaders& headers, const QByteArray& expected_digest) {
  const auto values = headers.values(QHttpHeaders::WellKnownHeader::Authorization);
  if (values.size() != 1) {
    return false;
  }
  constexpr QByteArrayView scheme = "Bearer ";
  const QByteArrayView authorization = values.first();
  if (authorization.size() <= scheme.size() ||
      authorization.first(scheme.size()).compare(scheme, Qt::CaseInsensitive) != 0) {
    return false;
  }
  const auto digest = TokenDigest(authorization.sliced(scheme.size()));
  unsigned char difference = 0;
  for (qsizetype i = 0; i < digest.size(); ++i) {
    difference |= static_cast<unsigned char>(digest[i] ^ expected_digest[i]);
  }
  return difference == 0;
}
} // namespace

RestServer::RestServer(const RestRouter& routes, QObject* parent, int timeout_ms,
                       std::optional<QByteArray> bearer_token)
    : QObject(parent), routes_(routes), timeout_ms_(timeout_ms),
      bearer_digest_(bearer_token ? std::optional{bearer_token->isEmpty() ? QByteArray{}
                                                                        : TokenDigest(*bearer_token)}
                                  : std::nullopt) {
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
  policy_error_.clear();
  if (bearer_digest_ && bearer_digest_->isEmpty()) {
    policy_error_ = tr("Bearer authentication requires a token.");
    return false;
  }
  if (!bearer_digest_ && !host.isLoopback()) {
    policy_error_ = tr("A listener without client authentication must use a loopback address.");
    return false;
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
  return policy_error_.isEmpty() ? tcp_.errorString() : policy_error_;
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
  if (bearer_digest_ && !MatchesBearer(request.headers(), *bearer_digest_)) {
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::WWWAuthenticate, "Bearer");
    reply->Fail(RestReply::StatusCode::Unauthorized, "unauthorized",
                tr("A valid bearer token is required."), std::move(headers));
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
