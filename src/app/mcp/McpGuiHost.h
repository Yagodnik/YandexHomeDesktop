#pragma once
#include <QObject>
#include <functional>
#include "mcp/McpBridge.h"
#include "worker/WorkerControlServer.h"

class McpGuiHost final : public QObject {
public:
  McpGuiHost(QString endpoint, HomeService* home, DeviceService* devices,
             ScenarioService* scenarios, std::function<bool()> authorized,
             std::function<quint16()> port, std::function<void(quint16)> set_port,
             QObject* parent = nullptr);
  bool Start();
  void Reset();

private:
  QString endpoint_;
  McpBridge bridge_;
  WorkerControlServer server_;
  std::function<bool()> authorized_;
  std::function<quint16()> port_;
  std::function<void(quint16)> set_port_;
};
