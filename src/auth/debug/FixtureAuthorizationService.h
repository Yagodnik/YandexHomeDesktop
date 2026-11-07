#pragma once

#include "auth/IAuthorizationService.h"

class FixtureAuthorizationService final : public IAuthorizationService {
public:
  using IAuthorizationService::IAuthorizationService;
  void AttemptLocalAuthorization() override;
  void AttemptAuthorization() override;
  void Logout() override;
  bool IsAuthorized() const override { return authorized_; }
  QString GetLastErrorCode() const override { return "0"; }
  std::optional<QString> GetToken() const override { return std::nullopt; }

private:
  bool authorized_ = true;
  bool logout_requested_ = false;
  quint64 generation_ = 0;
};
