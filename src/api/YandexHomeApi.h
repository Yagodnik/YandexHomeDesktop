#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include "model/UserInfo.h"

struct RequestFactory {
  inline static const QString kBearer = "Bearer ";

  static QNetworkRequest Create(const QString& endpoint, const QString& token);
};

class YandexHomeApi final : public QObject {
  Q_OBJECT
public:
  using TokenProvider = std::function<QString()>;
  explicit YandexHomeApi(TokenProvider token_provider, QObject *parent = nullptr);

  Q_INVOKABLE void RequestInfo();

signals:
  void infoReceived(const UserInfo& info);

private:
  using ReplyGuard = QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>;

  const QString kInfoEndpoint = "https://api.iot.yandex.net/v1.0/user/info";

  QNetworkAccessManager network_access_manager_;
  TokenProvider token_provider_;
};
