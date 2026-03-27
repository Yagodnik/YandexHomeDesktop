#pragma once

#include "IAuthorizationService.h"
#include "IOAuthSecrets.h"
#include "YandexOAuthSecrets.h"

class ISecretsStorage;

class YandexTokenAuthorizationService : public IAuthorizationService {
public:
  explicit YandexTokenAuthorizationService(QObject *parent = nullptr);

  Q_INVOKABLE void TryLoadTokenFromStorage() override;
  Q_INVOKABLE bool IsAuthorized() const override;
  Q_INVOKABLE void AttemptAuthorization(const QVariant& user_data) override;
  Q_INVOKABLE void SaveAuthToken(const QString& token) override;
  Q_INVOKABLE void Logout() override;
  Q_INVOKABLE QString GetLastErrorCode() const override;
  Q_INVOKABLE std::optional<QString> GetToken() const override;

private:
  std::unique_ptr<ISecretsStorage> secrets_storage_;
  std::unique_ptr<IOAuthSecrets> auth_secrets_;

  const QString kAppName = "com.artemyagodnik.YandexHomeDesktop_Test";
  const QString kSecureKey = "test_secret";

  QUrl code_req_url_;
  QUrl token_req_url_;
  int last_error_code_ = 0;
  std::optional<QString> token_;

  static QUrl GetAuthCodeUrl(const QString& base_url, const YandexOAuthSecrets::AuthSecrets& secrets);
  static QUrl GetAuthTokenUrl(const QString& base_url, const QString& code);
};
