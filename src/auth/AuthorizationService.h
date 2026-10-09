#pragma once

#include "IAuthorizationService.h"
#include "IAuthorizationFlow.h"
#include "ITokenStore.h"

// Dependencies outlive the service and deliver each result once, only while its
// QObject context is alive. The service owns session and operation ordering.
class AuthorizationService final : public IAuthorizationService {
  Q_OBJECT
public:
  explicit AuthorizationService(ITokenStore* store, IAuthorizationFlow* flow = nullptr,
                                QObject* parent = nullptr);
  void AttemptLocalAuthorization() override;
  void AttemptAuthorization() override;
  void Logout() override;
  [[nodiscard]] bool IsAuthorized() const override;
  [[nodiscard]] QString GetLastErrorCode() const override;
  [[nodiscard]] std::optional<QString> GetToken() const override;

private:
  enum class Operation { Idle, Reading, Authorizing, Writing, Deleting };
  void Fail(const AuthError& error);
  void DeleteToken();

  ITokenStore* store_;
  IAuthorizationFlow* flow_;
  Operation operation_ = Operation::Idle;
  quint64 generation_ = 0;
  bool logout_requested_ = false;
  quint32 last_error_code_ = 0;
  std::optional<QString> token_;
};
