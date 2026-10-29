#include "RestControlService.h"

#include <QUrl>

namespace {
QJsonObject Error(const QString& code, const QString& message) {
  return {{"ok", false}, {"error", QJsonObject{{"code", code}, {"message", message}}}};
}
} // namespace

RestControlService::RestControlService(Settings* settings, RestLaunchConfiguration configuration,
                                       QObject* parent)
    : QObject(parent), settings_(settings), configuration_(std::move(configuration)),
      worker_({configuration_.control_name, configuration_.executable, configuration_.arguments,
               "disable"},
              this) {
  connect(&worker_, &WorkerControlClient::finished, this, [this](QJsonObject result) {
    busy_ = false;
    if (!result["ok"].toBool()) {
      if (result["error"].toObject()["message"].toString().isEmpty()) {
        const auto code = result["error"].toObject()["code"].toString();
        const auto message = code == "timeout"         ? tr("Время управления REST API истекло.")
                             : code == "startup_error" ? tr("Не удалось запустить сервер REST API.")
                             : code == "invalid_response"
                                 ? tr("Некорректный ответ сервера REST API.")
                                 : tr("Не удалось подключиться к серверу REST API.");
        result = Error(code, message);
      }
    } else if (command_ == Command::Enable) {
      const auto port = result["port"].toInt();
      if (port < 1 || port > 65535) {
        result = Error("invalid_response", tr("Некорректный ответ сервера REST API."));
      } else if (requested_port_ && port != *requested_port_) {
        result = Error("port_conflict",
                       tr("REST API уже работает на другом порту. Сначала отключите его."));
      } else if (!configuration_.host.isEmpty() &&
                 result["host"].toString() != configuration_.host) {
        result = Error("host_conflict",
                       tr("REST API is already running on another host. Disable it first."));
      } else {
        settings_->SetRestPort(static_cast<quint16>(port));
        settings_->SetRestEnabled(true);
      }
    }
    if (result["ok"].toBool()) {
      const auto defaults = Status();
      for (auto it = defaults.begin(); it != defaults.end(); ++it) {
        if (!result.contains(it.key()))
          result.insert(it.key(), it.value());
      }
    }
    emit finished(result);
  });
}

bool RestControlService::IsBusy() const {
  return worker_.IsBusy();
}

void RestControlService::Request(Command command, std::optional<quint16> port, int timeout_ms) {
  if (busy_ && command != Command::Disable &&
      !(command_ == Command::Status && command == Command::Enable)) {
    return;
  }
  if (command == Command::Enable && ConflictsWithMcp(port.value_or(settings_->GetRestPort()))) {
    emit finished(Error("port_conflict", tr("Порты REST API и MCP-сервера должны различаться.")));
    return;
  }
  command_ = command;
  requested_port_ = port;
  busy_ = true;
  if (command == Command::Disable)
    settings_->SetRestEnabled(false);
  const auto worker_command = command == Command::Enable    ? WorkerControlClient::Command::Start
                              : command == Command::Disable ? WorkerControlClient::Command::Stop
                                                            : WorkerControlClient::Command::Status;
  const QStringList arguments{"--serve-rest", "--rest-port",
                              QString::number(port.value_or(settings_->GetRestPort())), "--timeout",
                              QString::number(timeout_ms)};
  worker_.Request(worker_command, {}, arguments, timeout_ms);
}

QJsonObject RestControlService::Status() const {
  const auto host = configuration_.host.isEmpty() ? QString("127.0.0.1") : configuration_.host;
  QUrl url;
  url.setScheme("http");
  url.setHost(host);
  url.setPort(settings_->GetRestPort());
  url.setPath("/v1");
  return {{"ok", true},
          {"enabled", settings_->GetRestEnabled()},
          {"running", false},
          {"fixture", configuration_.fixture},
          {"port", settings_->GetRestPort()},
          {"host", host},
          {"url", url.toString()}};
}
