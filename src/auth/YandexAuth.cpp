#include "YandexAuth.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

YandexAuth::YandexAuth(QObject *parent) : QObject{parent} {
  oauth2_.setReplyHandler(&reply_handler_);

  const auto auth_secrets_object = GetAuthSecrets();
  if (!auth_secrets_object.has_value()) {
    throw std::runtime_error("Can't load secrets");
  }

  auto auth_secrets = Serialization::From<AuthSecrets>(
    auth_secrets_object.value());

  qDebug() << "AuthUrl:" << auth_secrets.auth_url;
  qDebug() << "ClientSecret:" << auth_secrets.client_secret;

  oauth2_.setAuthorizationUrl({auth_secrets.auth_url});
  oauth2_.setTokenUrl({auth_secrets.access_token_url});
  oauth2_.setClientIdentifier(auth_secrets.client_id);
  oauth2_.setClientIdentifierSharedKey(auth_secrets.client_secret);

  QSet<QByteArray> scopeSet;
  for (const QString &scope : auth_secrets.scopes) {
    scopeSet.insert(scope.toUtf8());
  }
  oauth2_.setRequestedScopeTokens(scopeSet);
}

std::optional<QJsonObject> YandexAuth::GetAuthSecrets() const {
  QFile auth_secrets_file(kAuthSecretsPath);
  if (!auth_secrets_file.open(QIODevice::ReadOnly)) {
    return std::nullopt;
  }

  const QByteArray data = auth_secrets_file.readAll();
  auth_secrets_file.close();

  QJsonDocument document = QJsonDocument::fromJson(data);
  return document.object();
}
