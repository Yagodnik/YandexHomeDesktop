#pragma once

#include <QNetworkRequest>
#include <QString>

struct RequestFactory {
  inline static const QString kBearer = "Bearer ";
  inline static const QString kOAuth2 = "OAuth2 ";

  static QNetworkRequest CreateBearer(const QString& endpoint, const QString& token);
  static QNetworkRequest CreateOAuth2(const QString& endpoint, const QString& token);

private:
  static QNetworkRequest Create(
    const QString& endpoint,
    const QString& token,
    const QString& auth_prefix
  );
};
