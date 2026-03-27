#pragma once

#include "ISecretsStorage.h"

class QtKeyChainSecretsStorage : public ISecretsStorage {
public:
  QtKeyChainSecretsStorage(const QString& appName, const QString& secureKey, QObject *parent = nullptr);

  void TryWrite(const QString &token, WriteCallback callback) override;

  void TryRead(ReadCallback callback) override;

  void TryDelete(DeleteCallback callback) override;

private:
  QString appName_;
  QString secureKey_;
};
