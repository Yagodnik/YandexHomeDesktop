#pragma once

#include <QObject>
#include "auth/IAuthorizationService.h"

// Preserves the authorizationService QML API without exposing credentials.
class AuthorizationModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool authorizationPending READ authorizationPending NOTIFY authorizationStateChanged)
  Q_PROPERTY(QString authorizationCode READ authorizationCode NOTIFY authorizationStateChanged)
  Q_PROPERTY(QString authorizationUrl READ authorizationUrl NOTIFY authorizationStateChanged)
public:
  explicit AuthorizationModel(IAuthorizationService* service, QObject* parent = nullptr);
  Q_INVOKABLE void AttemptLocalAuthorization();
  Q_INVOKABLE void AttemptAuthorization();
  Q_INVOKABLE void Logout();
  Q_INVOKABLE void CancelAuthorization();
  bool authorizationPending() const { return service_->AuthorizationPending(); }
  QString authorizationCode() const { return service_->AuthorizationCode(); }
  QString authorizationUrl() const { return service_->AuthorizationUrl(); }
  Q_INVOKABLE bool IsAuthorized() const;
  Q_INVOKABLE QString GetLastErrorCode() const;

signals:
  void authorizationStateChanged();
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
