#pragma once

#include <QObject>
#include <QString>
#include <memory>

class RestServer;
class RestRouter;

// Loopback MCP transport. Tool operations are sent to the GUI's WorkerControl endpoint.
class McpHttpServer final : public QObject {
public:
  McpHttpServer(QString gui_endpoint, QByteArray token, QObject* parent = nullptr);
  ~McpHttpServer() override;
  bool Start(quint16 port);
  void Stop();
  quint16 Port() const;
  QString Error() const;

private:
  std::unique_ptr<RestRouter> routes_;
  std::unique_ptr<RestServer> server_;
};
