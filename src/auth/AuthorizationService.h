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

  Q_INVOKABLE [[nodiscard]] bool IsAuthorized() const;
  Q_INVOKABLE void AttemptAuthorization();
  Q_INVOKABLE void Logout();
  Q_INVOKABLE void TestWrite();
  Q_INVOKABLE void TestRead();
  Q_INVOKABLE void TestDelete();

signals:
  void authorized();
  void authorizationFailed();
  void initializationFailed();

private:
  const QString kAppName = "com.artemyagodnik.YandexHomeDesktop_Test";
  const QString kSecureKey = "test_secret";

  static constexpr int kDefaultPort = 1337;
  const QString kAuthSecretsPath = ":/auth/secrets.json";
  const QString kCallbackPath = ":/callback/index.html";

  QKeychain::WritePasswordJob write_job_;
  QKeychain::ReadPasswordJob read_job_;
  QKeychain::DeletePasswordJob delete_job_;

  JSON_STRUCT(AuthSecrets,
    (QString, auth_url),
    (QString, access_token_url),
    (QString, client_id),
    (QString, client_secret),
    (QString, redirect_base),
    (int, redirect_port),
    (QStringList, scopes)
  );

  [[nodiscard]] std::optional<QJsonObject> GetAuthSecrets() const;
  [[nodiscard]] static QSet<QByteArray> GetScopes(const QStringList& list);
  [[nodiscard]] bool PrepareCallbackPage();

  QOAuth2AuthorizationCodeFlow oauth2_;
  QOAuthHttpServerReplyHandler reply_handler_;

  std::optional<QString> token_;

private slots:
  void HandleAuthorizationStatus(QAbstractOAuth::Status status);
  static void AuthorizeWithBrowser(QUrl url);
};
