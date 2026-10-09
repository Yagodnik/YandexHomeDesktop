#pragma once

#include <QSet>
#include <QUrl>
#include "AuthResult.h"

struct OAuthConfiguration {
  QUrl authorization_url;
  QUrl token_url;
  QString client_id;
  QString client_secret;
  QUrl redirect_base;
  quint16 redirect_port = 0;
  QSet<QByteArray> scopes;

  static AuthResult<OAuthConfiguration> Parse(const QByteArray& json);
};
