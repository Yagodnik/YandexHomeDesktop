#pragma once

#include <QObject>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include "serialization/Serialization.h"

class YandexAuth final : public QObject {
public:
  explicit YandexAuth(QObject *parent = nullptr);

private:
  JSON_STRUCT(AuthSecrets,
    (QString, auth_url),
    (QString, access_token_url),
    (QString, client_id),
    (QString, client_secret),
    (QString, redirect_base),
    (int, redirect_port),
    (QStringList, scopes)
  );

  const QString kAuthSecretsPath = ":/auth/secrets.json";

  [[nodiscard]] std::optional<QJsonObject> GetAuthSecrets() const;

  QOAuth2AuthorizationCodeFlow oauth2_;
  QOAuthHttpServerReplyHandler reply_handler_;
};
