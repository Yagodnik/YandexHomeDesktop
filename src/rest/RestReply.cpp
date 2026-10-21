#include "RestReply.h"

#include <QCoreApplication>
#include <QJsonDocument>

RestReply::RestReply(QHttpServerResponder&& responder, QObject* parent, int timeout_ms)
    : QObject(parent), responder_(std::move(responder)) {
  timer_.setSingleShot(true);
  connect(&timer_, &QTimer::timeout, this, [this] {
    Fail(StatusCode::GatewayTimeout, "timeout",
         QCoreApplication::translate("RestServer", "Время выполнения запроса истекло."));
  });
  timer_.start(timeout_ms);
}

void RestReply::Send(QJsonObject result, StatusCode status) {
  if (!result.contains("ok")) {
    result["ok"] = true;
  }
  SendDocument(result, status);
}

void RestReply::SendDocument(const QJsonObject& document, StatusCode status) {
  if (finished_) {
    return;
  }
  finished_ = true;
  timer_.stop();

  QHttpHeaders headers;
  headers.append("Cache-Control", "no-store");
  // The QJsonDocument overload supplies Content-Type: application/json.
  responder_.write(QJsonDocument(document), headers, status);
  deleteLater();
}

void RestReply::Fail(StatusCode status, const QString& code, const QString& message) {
  Send({{"ok", false}, {"error", QJsonObject{{"code", code}, {"message", message}}}}, status);
}
