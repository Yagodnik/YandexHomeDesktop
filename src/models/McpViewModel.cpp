#include "McpViewModel.h"
#include "utils/Settings.h"

McpViewModel::McpViewModel(WorkerLaunchConfiguration configuration, Settings* settings, QObject* parent)
    : QObject(parent), worker_(std::move(configuration)), settings_(settings) {
  connect(&worker_, &WorkerControlClient::finished, this, [this](const QJsonObject& result) {
    busy_ = false;
    if (result.value("ok").toBool()) {
      running_ = result.value("running").toBool();
      url_ = result.value("url").toString();
    } else {
      error_ = tr("Не удалось управлять MCP-сервером. Проверьте вход и доступность порта.");
    }
    emit changed();
  });
  poll_.setInterval(2'000);
  connect(&poll_, &QTimer::timeout, this, &McpViewModel::Refresh);
  poll_.start();
  QTimer::singleShot(0, this, &McpViewModel::Refresh);
}
int McpViewModel::Port() const { return settings_->GetMcpPort(); }
void McpViewModel::SetPort(int port) {
  if (busy_ || running_ || port < 1 || port > 65535) return;
  if (port == settings_->GetRestPort()) {
    error_ = tr("Порты REST API и MCP-сервера должны различаться.");
    emit changed();
    return;
  }
  error_.clear();
  settings_->SetMcpPort(static_cast<quint16>(port));
  emit changed();
}
void McpViewModel::SetEnabled(bool enabled) {
  if (busy_ && enabled) return;
  if (enabled && settings_->GetMcpPort() == settings_->GetRestPort()) {
    error_ = tr("Порты REST API и MCP-сервера должны различаться.");
    emit changed();
    return;
  }
  error_.clear();
  busy_ = true;
  worker_.Request(enabled ? WorkerControlClient::Command::Start : WorkerControlClient::Command::Stop);
  emit changed();
}
void McpViewModel::Refresh() {
  if (!busy_) worker_.Request(WorkerControlClient::Command::Status);
}
