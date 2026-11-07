#include "McpBridge.h"

#include <QUuid>
#include <QJsonArray>
#include <utility>

namespace {
QJsonObject Error(const QString& message) { return {{"ok", false}, {"error", message}}; }
bool MatchesSchema(const QJsonValue& value, const QJsonObject& schema) {
  const auto type = schema.value("type").toString();
  if ((type == "object" && !value.isObject()) || (type == "string" && !value.isString()) ||
      (type == "boolean" && !value.isBool())) return false;
  const auto allowed = schema.value("enum").toArray();
  if (!allowed.isEmpty() && !allowed.contains(value)) return false;
  if (!value.isObject()) return true;
  const auto object = value.toObject();
  const auto properties = schema.value("properties").toObject();
  for (const auto& required : schema.value("required").toArray())
    if (!object.contains(required.toString())) return false;
  for (auto it = object.begin(); it != object.end(); ++it) {
    if (!properties.contains(it.key())) {
      if (schema.value("additionalProperties") == false) return false;
    } else if (!MatchesSchema(it.value(), properties.value(it.key()).toObject())) return false;
  }
  return true;
}
}

McpBridge::McpBridge(HomeService* home, DeviceService* devices, ScenarioService* scenarios,
                     QObject* parent)
    : QObject(parent), commands_(CreateMcpCommands(home, devices, scenarios)) {}

void McpBridge::Reset() {
  for (const auto& item : std::as_const(pending_)) delete item.context;
  pending_.clear();
}

void McpBridge::Finish(const QString& id, QJsonObject result) {
  auto it = pending_.find(id);
  if (it == pending_.end()) return;
  it->result = std::move(result);
  it->finished = true;
}

QJsonObject McpBridge::Handle(const QJsonObject& request) {
  const auto command = request.value("command").toString();
  if (command == "tools") return {{"ok", true}, {"tools", DescribeMcpCommands(commands_)}};
  if (command == "poll") {
    const auto id = request.value("id").toString();
    auto it = pending_.find(id);
    if (it == pending_.end()) return Error("Unknown or expired request.");
    if (!it->finished) return {{"ok", true}, {"pending", true}};
    auto result = it->result;
    delete it->context;
    pending_.erase(it);
    return {{"ok", true}, {"pending", false}, {"result", result}};
  }
  if (command != "submit") return Error("Unknown command.");
  const auto tool = request.value("tool").toString();
  if (!request.value("arguments").isObject()) return Error("Arguments must be an object.");
  const auto definition = commands_.value(tool);
  if (!definition.command) return Error("Unknown tool.");
  if (!MatchesSchema(request.value("arguments"), definition.input_schema)) return Error("Invalid arguments.");
  if (pending_.size() >= 64) return Error("Too many pending requests.");
  const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  auto* context = new QObject(this);
  pending_.insert(id, {context, {}, false});
  QTimer::singleShot(60'000, context, [this, id] {
    auto it = pending_.find(id);
    if (it != pending_.end()) { it->context->deleteLater(); pending_.erase(it); }
  });
  definition.command->Execute(request.value("arguments").toObject(), context,
                     [this, id](QJsonObject result) { Finish(id, std::move(result)); });
  return {{"ok", true}, {"id", id}};
}
