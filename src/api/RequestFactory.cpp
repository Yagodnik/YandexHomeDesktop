#include "RequestFactory.h"

QNetworkRequest RequestFactory::CreateBearer(const QString &endpoint, const QString &token) {
  return Create(endpoint, token, kBearer);
}

QNetworkRequest RequestFactory::CreateOAuth2(const QString &endpoint, const QString &token) {
  return Create(endpoint, token, kOAuth2);
}

QNetworkRequest RequestFactory::Create(
  const QString &endpoint, const QString &token, const QString &auth_prefix
) {
  QNetworkRequest request(endpoint);
  const QString token_string = auth_prefix + token;
  const QByteArray token_bytes = token_string.toUtf8();

  request.setRawHeader("User-Agent", "YandexHomeApi");
  request.setRawHeader("Authorization", token_bytes);

  return request;
}
