#pragma once

#include <QObject>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>

#include "IAuthorizationService.h"
#include "ISecretsStorage.h"
#include "YandexOAuthSecrets.h"
#include "serialization/Serialization.h"

class WebAuthorizationService : public IAuthorizationService {
  Q_OBJECT
public:
  explicit WebAuthorizationService(QObject *parent = nullptr);

  Q_INVOKABLE void TryLoadTokenFromStorage() override;
  Q_INVOKABLE [[nodiscard]] bool IsAuthorized() const override;
  Q_INVOKABLE void AttemptAuthorization(const QVariant& user_data) override;
  Q_INVOKABLE void SaveAuthToken(const QString &token) override;
  Q_INVOKABLE void Logout() override;
  Q_INVOKABLE QString GetLastErrorCode() const override;

  [[nodiscard]] std::optional<QString> GetToken() const override;

private:
  std::unique_ptr<ISecretsStorage> secrets_storage_;
  std::unique_ptr<IOAuthSecrets> auth_secrets_;

  const QString kAppName = "com.artemyagodnik.YandexHomeDesktop_Test";
  const QString kSecureKey = "test_secret";

  static constexpr int kDefaultPort = 1337;
  const QString kCallbackPath = ":/callback/index.html";

  QOAuth2AuthorizationCodeFlow oauth2_;
  QOAuthHttpServerReplyHandler reply_handler_;
  int last_error_code_{0};
  std::optional<QString> token_;

  [[nodiscard]] static QSet<QByteArray> GetScopes(const QStringList& list);
  [[nodiscard]] bool PrepareCallbackPage();

private slots:
  void HandleAuthorizationStatus(QAbstractOAuth::Status status);
  static void AuthorizeWithBrowser(QUrl url);
};