#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QImage>

#include "serialization/Serialization.h"

class YandexAccount : public QObject {
  Q_OBJECT
public:
  using TokenProvider = std::function<QString()>;
  explicit YandexAccount(TokenProvider token_provider, QObject *parent = nullptr);

  Q_INVOKABLE void LoadData();
  Q_INVOKABLE [[nodiscard]] QString GetName() const;
  Q_INVOKABLE [[nodiscard]] QString GetAvatarUrl() const;
  Q_INVOKABLE [[nodiscard]] QString GetEmail() const;

signals:
  void dataLoaded();
  void dataLoadingFailed();

private:
  JSON_STRUCT(AccountInfo,
    (QString, login),
    (QString, display_name),
    (QString, default_avatar_id),
    (QString, default_email)
  );

  const QString kAccountInfoEndpoint = "https://login.yandex.ru/info";
  const QString kAvatarUrl = "https://avatars.yandex.net/get-yapic/%1/";

  QNetworkAccessManager network_manager_;

  QString name_;
  QString avatar_id_;
  QString email_;

  TokenProvider token_provider_;
};
