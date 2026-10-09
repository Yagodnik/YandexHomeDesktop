#include "OAuthConfiguration.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

AuthResult<OAuthConfiguration> OAuthConfiguration::Parse(const QByteArray& json) {
  const auto invalid = std::unexpected(AuthError{AuthErrorKind::Initialization, 0x80010001u, {}});
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(json, &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) { return invalid; }
  const auto object = document.object();
  for (const auto* key : {"auth_url", "access_token_url", "client_id", "redirect_base"}) {
    if (!object[key].isString() || object[key].toString().trimmed().isEmpty()) { return invalid; }
  }
  if (!object["client_secret"].isString() || !object["redirect_port"].isDouble() ||
      !object["scopes"].isArray()) { return invalid; }
  const double port = object["redirect_port"].toDouble();
  if (port < 1 || port > 65535 || std::floor(port) != port) { return invalid; }

  OAuthConfiguration config;
  config.authorization_url = QUrl(object["auth_url"].toString(), QUrl::StrictMode);
  config.token_url = QUrl(object["access_token_url"].toString(), QUrl::StrictMode);
  config.client_id = object["client_id"].toString();
  config.client_secret = object["client_secret"].toString();
  config.redirect_base = QUrl(object["redirect_base"].toString(), QUrl::StrictMode);
  config.redirect_port = static_cast<quint16>(port);
  for (const auto& url : {config.authorization_url, config.token_url}) {
    if (!url.isValid() || url.scheme() != "https" || url.host().isEmpty() ||
        !url.userInfo().isEmpty() || url.hasFragment()) { return invalid; }
  }
  const auto& redirect = config.redirect_base;
  if (!redirect.isValid() || redirect.scheme() != "http" ||
      (redirect.host() != "127.0.0.1" && redirect.host() != "localhost" && redirect.host() != "::1") ||
      !redirect.userInfo().isEmpty() || redirect.port() != -1 ||
      redirect.hasQuery() || redirect.hasFragment()) { return invalid; }
  for (const auto& scope : object["scopes"].toArray()) {
    if (!scope.isString() || scope.toString().contains(' ')) { return invalid; }
    if (!scope.toString().isEmpty()) { config.scopes.insert(scope.toString().toUtf8()); }
  }
  return config;
}
