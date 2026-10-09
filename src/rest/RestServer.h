#pragma once

#include "yh/apprest_export.h"

#include <QHttpServer>
#include <QObject>
#include <QTcpServer>

#include "RestProtocol.h"

class RestRouter;

// HTTP listener, access control, and ownership of in-flight replies.
class APPREST_EXPORT RestServer final : public QObject {
  Q_OBJECT
public:
  explicit RestServer(const RestRouter& routes, QObject* parent = nullptr,
    int timeout_ms = RestProtocol::DefaultTimeoutMs);
  ~RestServer() override;
  bool Start(quint16 port);
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
};
