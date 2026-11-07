#pragma once

#include <QHash>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include <memory>

class QObject;
class HomeService;
class DeviceService;
class ScenarioService;

class McpCommand {
public:
  using Completion = std::function<void(QJsonObject)>;
  virtual ~McpCommand() = default;
  virtual void Execute(const QJsonObject& arguments, QObject* context, Completion completion) const = 0;
};

struct McpTool {
  QString description;
  QJsonObject input_schema;
  std::shared_ptr<const McpCommand> command;
};
using McpCommands = QHash<QString, McpTool>;
McpCommands CreateMcpCommands(HomeService* home, DeviceService* devices, ScenarioService* scenarios);
QJsonArray DescribeMcpCommands(const McpCommands& commands);
