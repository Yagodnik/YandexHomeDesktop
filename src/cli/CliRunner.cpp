#include "CliRunner.h"
#include <QJsonArray>
#include <QJsonDocument>

namespace {
QString ApiKind(ApiErrorKind kind) {
  switch (kind) {
    case ApiErrorKind::Network: return "network";
    case ApiErrorKind::Timeout: return "timeout";
    case ApiErrorKind::Http: return "http";
    case ApiErrorKind::InvalidResponse: return "invalid_response";
    case ApiErrorKind::Service: return "service";
  }
  return "unknown";
}
}

CliRunner::CliRunner(CliServices services, Writer writer, QObject* parent)
  : QObject(parent), services_(std::move(services)), writer_(std::move(writer)) {
  timer_.setSingleShot(true);
  connect(&timer_, &QTimer::timeout, this, &CliRunner::OnTimeout);
}
void CliRunner::Start(const CliCommand& command) {
  if (operation_) { return; }
  command_ = command;
  operation_ = std::make_unique<QObject>();
  timer_.start(command.timeout_ms);
  command.operation->Execute(*this);
}
QObject* CliRunner::Owner() const { return operation_.get(); }
const CliServices& CliRunner::Services() const { return services_; }

QByteArray CliRunner::FormatError(bool json, const QString& code, const QString& message, const QJsonObject& details) {
  if (!json) {
    QString text = tr("Ошибка [%1]: %2\n").arg(code, message);
    for (const auto& item : details["matches"].toArray()) {
      const auto target = item.toObject();
      text += QString("  %1\t%2\t%3\n").arg(target["id"].toString(), target["name"].toString(), target["household_id"].toString());
    }
    return text.toUtf8();
  }
  QJsonObject error{{"code", code}, {"message", message}};
  for (auto item = details.begin(); item != details.end(); ++item) { error[item.key()] = item.value(); }
  return QJsonDocument(QJsonObject{{"ok", false}, {"error", error}}).toJson(QJsonDocument::Compact) + '\n';
}

void CliRunner::FailApi(const ApiError& error) {
  const auto exit_code = error.kind == ApiErrorKind::Timeout ? Timeout :
    error.http_status == 401 || error.http_status == 403 ? Unauthorized : error.http_status == 404 ? NotFound : RequestFailed;
  Fail(exit_code, error.kind == ApiErrorKind::Timeout ? "timeout" : "api_error", error.message,
    {{"kind", ApiKind(error.kind)}, {"http_status", error.http_status}});
}
void CliRunner::Fail(int exit_code, const QString& code, const QString& message, const QJsonObject& details) {
  Finish(exit_code, FormatError(command_.json, code, message, details));
}
void CliRunner::Complete(const QJsonObject& output, const QString& text) {
  auto result = output;
  result["ok"] = true;
  Finish(Success, command_.json ? QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n' : text.toUtf8());
}
void CliRunner::Finish(int exit_code, const QByteArray& output) {
  if (!operation_) { return; }
  timer_.stop();
  operation_.reset();
  writer_(output, exit_code != Success);
  emit finished(exit_code);
}
void CliRunner::OnTimeout() { Fail(Timeout, "timeout", tr("Время выполнения команды истекло.")); }
