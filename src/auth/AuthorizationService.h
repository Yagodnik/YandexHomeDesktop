#pragma once

#include <QObject>
#include <QJsonObject>
#include <optional>
#include <memory>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include "qtkeychain/keychain.h"

class AuthorizationService : public QObject {
  Q_OBJECT
public:
  explicit AuthorizationService(QObject *parent = nullptr, bool use_fake_api = false,
                                bool interactive = true);

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
  void logoutFinished();
  void logoutFailed(const QString &error);
  void authorizationFailed();
  void initializationFailed();
  void authorizationCanceled();

private:
  const QString kAppName = "com.artemyagodnik.YandexHomeDesktop_Test";
  const QString kSecureKey = "test_secret";

  static constexpr int kDefaultPort = 1337;
  const QString kAuthSecretsPath = ":/auth/secrets.json";
  const QString kCallbackPath = ":/callback/index.html";

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
  std::unique_ptr<QOAuthHttpServerReplyHandler> reply_handler_;
  bool use_fake_api_ = false;
  bool fixture_authorized_ = true;
  int last_error_code_{0};
  std::optional<QString> token_;

private slots:
  void HandleAuthorizationStatus(QAbstractOAuth::Status status);
  static void AuthorizeWithBrowser(QUrl url);
};
