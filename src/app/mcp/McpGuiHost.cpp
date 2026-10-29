#include "McpGuiHost.h"

McpGuiHost::McpGuiHost(QString endpoint, HomeService* home, DeviceService* devices,
                       ScenarioService* scenarios, std::function<bool()> authorized,
                       std::function<quint16()> port, std::function<void(quint16)> set_port,
                       QObject* parent)
    : QObject(parent), endpoint_(std::move(endpoint)), bridge_(home, devices, scenarios),
      authorized_(std::move(authorized)), port_(std::move(port)), set_port_(std::move(set_port)) {
  server_.SetHandler([this](const QJsonObject& request) {
    if (request.value("command") == "status")
      return QJsonObject{{"ok", true}, {"running", authorized_()}, {"port", port_()}};
    if (!authorized_())
      return QJsonObject{{"ok", false}, {"error", "GUI is not authorized."}};
    if (request.value("command") == "set_port") {
      const auto value = request.value("port");
      const auto port = value.toInt();
      if (!value.isDouble() || port < 1 || port > 65535 || value.toDouble() != port)
        return QJsonObject{{"ok", false}, {"error", "Invalid port."}};
      set_port_(static_cast<quint16>(port));
      return QJsonObject{{"ok", true}, {"port", port}};
    }
    return bridge_.Handle(request);
  });
}
bool McpGuiHost::Start() {
  return server_.Start(endpoint_);
}
void McpGuiHost::Reset() {
  bridge_.Reset();
}
