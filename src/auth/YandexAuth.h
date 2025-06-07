#pragma once

#include <QObject>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include "serialization/Serialization.h"

class YandexAuth : public QObject {
  Q_OBJECT
public:
  explicit YandexAuth(QObject *parent = nullptr);

  Q_INVOKABLE [[nodiscard]] bool IsAuthorized() const;
  Q_INVOKABLE void AttemptAuthorization();

signals:
  void authorized();
  void authorizationFailed();
  void initializationFailed();

private:
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
