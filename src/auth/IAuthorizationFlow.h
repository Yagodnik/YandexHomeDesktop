#pragma once

#include "AuthResult.h"

class QObject;

class IAuthorizationFlow {
public:
  using PromptHandler = std::function<void(const QString&, const QString&)>;
  virtual void SetPromptHandler(PromptHandler) {}
  virtual ~IAuthorizationFlow() = default;
  // Deliver once while context is alive. Cancel releases browser-flow resources
  // and suppresses completion of the canceled attempt.
  virtual void Start(QObject* context, AuthResultHandler<QString> handler) = 0;
  virtual void Cancel() = 0;
};
