#pragma once

#include <QByteArray>
#include <QNetworkRequest>
#include <QObject>

#include "ApiResult.h"

struct HttpResponse {
  QByteArray body;
  int status_code = 0;
};

class IHttpTransport {
public:
  virtual ~IHttpTransport() = default;
  virtual void Get(const QNetworkRequest& request, QObject* context,
                   ApiResultHandler<HttpResponse> handler) = 0;
  virtual void Post(const QNetworkRequest& request, const QByteArray& body, QObject* context,
                    ApiResultHandler<HttpResponse> handler) = 0;
};
