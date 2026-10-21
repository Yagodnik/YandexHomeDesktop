#pragma once

#include "AuthResult.h"

class QObject;

// Deliver exactly one result while context is alive. Writes/deletes finish only
// after the storage mutation completes; the service orders these operations.
class ITokenStore {
public:
  virtual ~ITokenStore() = default;
  virtual void Read(QObject* context, AuthResultHandler<QString> handler) = 0;
  virtual void Write(const QString& token, QObject* context, AuthResultHandler<void> handler) = 0;
  virtual void Delete(QObject* context, AuthResultHandler<void> handler) = 0;
};
