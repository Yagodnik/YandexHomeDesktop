#pragma once

#include <QNetworkAccessManager>

#include "IAuthorizationService.h"
#include "IOAuthSecrets.h"
#include "ISecretsStorage.h"
#include "YandexOAuthSecrets.h"

class YandexTokenAuthorizationService : public IAuthorizationService {
public:
  explicit YandexTokenAuthorizationService(QNetworkAccessManager* network_manager, QObject *parent = nullptr);

  void TryLoadTokenFromStorage() override;
  Q_INVOKABLE [[nodiscard]] bool IsAuthorized() const override;
  Q_INVOKABLE void AttemptAuthorization(const QVariant& user_data) override;
  Q_INVOKABLE void SaveAuthToken(const QString& token) override;
  Q_INVOKABLE void Logout() override;
  Q_INVOKABLE [[nodiscard]] QString GetLastErrorCode() const override;
  Q_INVOKABLE [[nodiscard]] std::optional<QString> GetToken() const override;

private:
  JSON_STRUCT(AuthResponse,
    (QString, token_type),
    (QString, access_token),
    (uint64_t, expires_at),
    (QString, refresh_token),
    (QString, scope)
  );

  QNetworkAccessManager* network_manager_;
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
