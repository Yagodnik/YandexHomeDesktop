#pragma once

#include "yh/apprest_export.h"

#include <QHttpServerResponder>
#include <QJsonObject>
#include <QObject>
#include <QTimer>

// The reply is also the callback context: expiration cancels further delivery.
class APPREST_EXPORT RestReply final : public QObject {
public:
  using StatusCode = QHttpServerResponder::StatusCode;

  RestReply(QHttpServerResponder&& responder, QObject* parent, int timeout_ms);

  void Send(QJsonObject result, StatusCode status = StatusCode::Ok);
  void Fail(StatusCode status, const QString& code, const QString& message);

private:
  QHttpServerResponder responder_;
  QTimer timer_;
  bool finished_ = false;
};
