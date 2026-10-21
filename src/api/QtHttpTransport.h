#pragma once

#include "yh/yandexapiadapter_export.h"

#include <QNetworkAccessManager>

#include "IHttpTransport.h"

class QNetworkReply;

class YANDEXAPIADAPTER_EXPORT QtHttpTransport final : public QObject, public IHttpTransport {
  Q_OBJECT
public:
  // An injected manager must outlive the transport.
  explicit QtHttpTransport(QObject* parent = nullptr, int timeout_ms = 5000,
                           QNetworkAccessManager* manager = nullptr);

  void Get(const QNetworkRequest& request, QObject* context,
           ApiResultHandler<HttpResponse> handler) override;
  void Post(const QNetworkRequest& request, const QByteArray& body, QObject* context,
            ApiResultHandler<HttpResponse> handler) override;

private:
  void WatchReply(QNetworkReply* reply, QObject* context, ApiResultHandler<HttpResponse> handler);

  QNetworkAccessManager default_manager_;
  QNetworkAccessManager* manager_;
  int timeout_ms_;
};
