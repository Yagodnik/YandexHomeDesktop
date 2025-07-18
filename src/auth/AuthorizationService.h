#pragma once

#include <QObject>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include "serialization/Serialization.h"
#include "qtkeychain/keychain.h"

class AuthorizationService : public QObject {
  Q_OBJECT
public:
  explicit AuthorizationService(QObject *parent = nullptr);

  Q_INVOKABLE void AttemptLocalAuthorization();
  Q_INVOKABLE [[nodiscard]] bool IsAuthorized() const;
  Q_INVOKABLE void AttemptAuthorization();
  Q_INVOKABLE void Logout();
  Q_INVOKABLE QString GetLastErrorCode() const;
  [[nodiscard]] std::optional<QString> GetToken() const;

signals:
  void authorized();
  void unauthorized();
  void logout();
  void authorizationFailed();
  void initializationFailed();
  void authorizationCanceled();

private:
  const QString kAppName = "com.artemyagodnik.YandexHomeDesktop_Test";
  const QString kSecureKey = "test_secret";

  static constexpr int kDefaultPort = 1337;
  const QString kAuthSecretsPath = ":/auth/secrets.json";
  const QString kCallbackPath = ":/callback/index.html";

  JSON_STRUCT(AuthSecrets,
    (QString, auth_url),
    (QString, access_token_url),
    (QString, client_id),
    (QString, client_secret),
    (QString, redirect_base),
    (int, redirect_port),
    (QStringList, scopes)
  );

  void TryWrite(const QString& key);
  void TryRead();
  void TryDelete();

  [[nodiscard]] std::optional<QJsonObject> GetAuthSecrets() const;
  [[nodiscard]] static QSet<QByteArray> GetScopes(const QStringList& list);
  [[nodiscard]] bool PrepareCallbackPage();

  void ReadTokenHandler(QKeychain::ReadPasswordJob *job);
  void WriteTokenHandler(QKeychain::WritePasswordJob *job);
  void DeleteTokenHandler(QKeychain::DeletePasswordJob *job);

  QOAuth2AuthorizationCodeFlow oauth2_;
  QOAuthHttpServerReplyHandler reply_handler_;
  int last_error_code_{0};
  std::optional<QString> token_;

private slots:
  void HandleAuthorizationStatus(QAbstractOAuth::Status status);
  static void AuthorizeWithBrowser(QUrl url);
};
