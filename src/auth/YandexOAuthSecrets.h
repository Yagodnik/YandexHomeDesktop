#pragma once

#include "IOAuthSecrets.h"

class YandexOAuthSecrets final : public IOAuthSecrets {
public:
  explicit YandexOAuthSecrets(QObject *parent = nullptr);

  [[nodiscard]] std::optional<AuthSecrets> GetAuthSecrets() const override;

private:
  const QString kAuthSecretsPath = ":/auth/secrets.json";
  std::optional<AuthSecrets> authSecrets_;
};
