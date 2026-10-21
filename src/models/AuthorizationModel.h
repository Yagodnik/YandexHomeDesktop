#pragma once

#include <QObject>
#include "auth/IAuthorizationService.h"

// Preserves the authorizationService QML API without exposing credentials.
class AuthorizationModel final : public QObject {
  Q_OBJECT
public:
  explicit AuthorizationModel(IAuthorizationService* service, QObject* parent = nullptr);
  Q_INVOKABLE void AttemptLocalAuthorization();
  Q_INVOKABLE void AttemptAuthorization();
  Q_INVOKABLE void Logout();
  Q_INVOKABLE bool IsAuthorized() const;
  Q_INVOKABLE QString GetLastErrorCode() const;

signals:
  void authorized();
  void unauthorized();
  void logout();
  void logoutFinished();
  void logoutFailed(const QString& error);
  void authorizationFailed();
  void initializationFailed();
  void authorizationCanceled();

private:
  IAuthorizationService* service_;
};
