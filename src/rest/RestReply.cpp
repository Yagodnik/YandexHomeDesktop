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
  if (finished_) {
    return;
  }
  finished_ = true;
  timer_.stop();
  if (!result.contains("ok")) {
    result["ok"] = true;
  }

  QHttpHeaders headers;
  headers.append("Content-Type", "application/json");
  headers.append("Cache-Control", "no-store");
  responder_.write(QJsonDocument(result), headers, status);
  deleteLater();
}

void RestReply::Fail(StatusCode status, const QString& code, const QString& message) {
  Send({{"ok", false}, {"error", QJsonObject{{"code", code}, {"message", message}}}}, status);
}
