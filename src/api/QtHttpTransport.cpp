#include "QtHttpTransport.h"

#include <QNetworkReply>
#include <QPointer>
#include <QTimer>
#include <memory>

QtHttpTransport::QtHttpTransport(QObject* parent, int timeout_ms, QNetworkAccessManager* manager)
  : QObject(parent), manager_(manager ? manager : &default_manager_), timeout_ms_(timeout_ms) {}

void QtHttpTransport::Get(const QNetworkRequest& request, QObject* context,
                          ApiResultHandler<HttpResponse> handler) {
  WatchReply(manager_->get(request), context, std::move(handler));
}

void QtHttpTransport::Post(const QNetworkRequest& request, const QByteArray& body,
                           QObject* context, ApiResultHandler<HttpResponse> handler) {
  WatchReply(manager_->post(request, body), context, std::move(handler));
}

void QtHttpTransport::WatchReply(QNetworkReply* reply, QObject* context,
                                 ApiResultHandler<HttpResponse> handler) {
  QPointer<QObject> owner(context);
  auto delivered = std::make_shared<bool>(false);
  auto* timer = new QTimer(reply);
  timer->setSingleShot(true);
  timer->start(timeout_ms_);

  connect(timer, &QTimer::timeout, reply, [reply, owner, handler, delivered]() {
    if (!reply->isRunning() || *delivered) {
      return;
    }
    *delivered = true;
    reply->abort();
    if (owner) {
      handler(std::unexpected(ApiError{ApiErrorKind::Timeout, "Timeout reached!"}));
    }
  });

  connect(reply, &QNetworkReply::finished, reply, [reply, owner, handler, delivered]() {
    reply->deleteLater();
    if (!owner || *delivered) {
      return;
    }
    *delivered = true;

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 0 && (status < 200 || status >= 300)) {
      handler(std::unexpected(ApiError{ApiErrorKind::Http,
                                       QString("HTTP %1").arg(status), status}));
      return;
    }

    if (reply->error() != QNetworkReply::NoError) {
      handler(std::unexpected(ApiError{ApiErrorKind::Network, reply->errorString()}));
      return;
    }

    if (status == 0) {
      handler(std::unexpected(ApiError{ApiErrorKind::InvalidResponse,
                                       "Missing HTTP status"}));
      return;
    }

    handler(HttpResponse{reply->readAll(), status});
  });
}
