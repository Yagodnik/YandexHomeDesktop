#pragma once

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include "McpCommand.h"

class HomeService;
class DeviceService;
class ScenarioService;

// The control channel is synchronous, whereas application services are not.
// Submit creates a bounded request and Poll retrieves its eventual result.
class McpBridge final : public QObject {
public:
  McpBridge(HomeService* home, DeviceService* devices, ScenarioService* scenarios,
            QObject* parent = nullptr);
  QJsonObject Handle(const QJsonObject& request);
  void Reset();

private:
  struct Pending { QObject* context; QJsonObject result; bool finished = false; };
  void Finish(const QString& id, QJsonObject result);
  McpCommands commands_;
  QHash<QString, Pending> pending_;
};
