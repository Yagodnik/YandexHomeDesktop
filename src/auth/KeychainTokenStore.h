#pragma once

#include "yh/yandexauth_export.h"

#include <QObject>
#include "ITokenStore.h"

class YANDEXAUTH_EXPORT KeychainTokenStore final : public QObject, public ITokenStore {
public:
  using QObject::QObject;
  void Read(QObject* context, AuthResultHandler<QString> handler) override;
  void Write(const QString& token, QObject* context, AuthResultHandler<void> handler) override;
  void Delete(QObject* context, AuthResultHandler<void> handler) override;
};
