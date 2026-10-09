#pragma once

#include "yh/yandexapiadapter_export.h"

#include <functional>

#include "IAccountApi.h"
#include "IHttpTransport.h"

class YANDEXAPIADAPTER_EXPORT YandexAccountApi final : public QObject, public IAccountApi {
  Q_OBJECT
public:
  using TokenProvider = std::function<QString()>;
  explicit YandexAccountApi(TokenProvider token_provider, IHttpTransport* transport,
                            QObject* parent = nullptr);

  void LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) override;

private:
  const QString kAccountInfoEndpoint = "https://login.yandex.ru/info";
  TokenProvider token_provider_;
  IHttpTransport* transport_;
};
