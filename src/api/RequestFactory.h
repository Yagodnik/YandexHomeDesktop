#pragma once

#include "yh/yandexapiadapter_export.h"

#include <QNetworkRequest>
#include <QString>
#include "ApiResult.h"

struct YANDEXAPIADAPTER_EXPORT RequestFactory {
  inline static const QString kBearer = "Bearer ";
  inline static const QString kOAuth2 = "OAuth2 ";

  static QNetworkRequest CreatePlain(const QString& endpoint);
  static QNetworkRequest CreateBearer(const QString& endpoint, const QString& token);
  static ApiResult<QNetworkRequest> CreateAuthorizedBearer(const QString& endpoint, const QString& token);
  static QNetworkRequest CreateOAuth2(const QString& endpoint, const QString& token);

private:
  static QNetworkRequest Create(
    const QString& endpoint,
    const QString& token,
    const QString& auth_prefix
  );
};
