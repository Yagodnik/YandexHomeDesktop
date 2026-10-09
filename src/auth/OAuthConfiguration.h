#pragma once

#include "yh/yandexoauth_export.h"

#include <QSet>
#include <QUrl>
#include "AuthResult.h"

struct YANDEXOAUTH_EXPORT OAuthConfiguration {
  QUrl authorization_url;
  QUrl token_url;
  QString client_id;
  QString client_secret;
  QUrl redirect_base;
  quint16 redirect_port = 0;
  QSet<QByteArray> scopes;

  static AuthResult<OAuthConfiguration> Parse(const QByteArray& json);
};
