#pragma once

#include "yh/apprest_export.h"

#include <QHttpServer>
#include <QObject>
#include <QTcpServer>
#include <optional>

#include "RestProtocol.h"

class RestRouter;

// HTTP listener, access control, and ownership of in-flight replies.
class APPREST_EXPORT RestServer final : public QObject {
  Q_OBJECT
public:
  explicit RestServer(const RestRouter& routes, QObject* parent = nullptr,
                      int timeout_ms = RestProtocol::DefaultTimeoutMs,
                      std::optional<QByteArray> bearer_token = std::nullopt);
  ~RestServer() override;
  bool Start(quint16 port);
  bool Start(quint16 port, const QHostAddress& host);
  void Stop();
  quint16 Port() const;
  QString Error() const;

private:
  void Handle(const QHttpServerRequest& request, QHttpServerResponder& responder);

  const RestRouter& routes_;
  QHttpServer http_;
  QTcpServer tcp_;
  QObject requests_;
  int timeout_ms_;
  const std::optional<QByteArray> bearer_digest_;
  QString policy_error_;
};
