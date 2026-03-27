#include "YandexOAuthSecrets.h"

#include <QFile>

YandexOAuthSecrets::YandexOAuthSecrets(QObject *parent) : IOAuthSecrets(parent) {
  QFile auth_secrets_file(kAuthSecretsPath);
  if (!auth_secrets_file.open(QIODevice::ReadOnly)) {
    authSecrets_ = std::nullopt;
  }

  const QByteArray data = auth_secrets_file.readAll();
  auth_secrets_file.close();

  const QJsonDocument document = QJsonDocument::fromJson(data);
  authSecrets_ = Serialization::From<AuthSecrets>(document.object());
}

std::optional<YandexOAuthSecrets::AuthSecrets> YandexOAuthSecrets::GetAuthSecrets() const {
  return authSecrets_;
}
