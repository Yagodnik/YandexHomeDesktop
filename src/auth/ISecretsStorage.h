#pragma once

#include <expected>
#include <QObject>

#include "qtkeychain/keychain.h"

class ISecretsStorage : public QObject {
  Q_OBJECT
public:
  explicit ISecretsStorage(QObject *parent = nullptr) : QObject(parent) {}

  struct Error {
    int errorCode;
    QString errorText;
  };

  using ReadCallback = std::function<void(std::expected<QString, ISecretsStorage::Error>)>;
  using WriteCallback = std::function<void(std::optional<ISecretsStorage::Error>)>;
  using DeleteCallback = std::function<void(std::optional<ISecretsStorage::Error>)>;

  virtual void TryWrite(const QString& token, WriteCallback callback) = 0;
  virtual void TryRead(ReadCallback callback) = 0;
  virtual void TryDelete(DeleteCallback callback) = 0;
};
