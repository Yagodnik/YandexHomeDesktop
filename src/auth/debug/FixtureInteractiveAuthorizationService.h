#pragma once

#include "auth/IAuthorizationService.h"
#include <QTcpServer>

// Debug-only browser simulation. All state is in memory and the listener is loopback-only.
class FixtureInteractiveAuthorizationService final : public IAuthorizationService {
public:
  explicit FixtureInteractiveAuthorizationService(QObject* parent = nullptr);
  void AttemptLocalAuthorization() override;
  void AttemptAuthorization() override;
  void CancelAuthorization() override;
  void Logout() override;
  bool IsAuthorized() const override {
    return authorized_;
  }
  bool AuthorizationPending() const override {
    return pending_;
  }
  QString AuthorizationCode() const override {
    return pending_ ? QStringLiteral("DEMO-1234") : QString{};
  }
  QString AuthorizationUrl() const override;
  QString GetLastErrorCode() const override {
    return "0";
  }
  std::optional<QString> GetToken() const override {
    return std::nullopt;
  }

private:
  void AcceptConnections();
  void Reply(QTcpSocket* socket, const QByteArray& request);
  void Stop();
  QTcpServer server_;
  bool authorized_ = false;
  bool pending_ = false;
  bool logout_requested_ = false;
  quint64 generation_ = 0;
};
