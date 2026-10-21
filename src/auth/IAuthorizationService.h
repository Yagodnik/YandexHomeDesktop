#pragma once

#include "yh/authcontracts_export.h"

#include <QObject>
#include <QString>
#include <optional>

class AUTHCONTRACTS_EXPORT IAuthorizationService : public QObject {
  Q_OBJECT
public:
  using QObject::QObject;
  virtual void AttemptLocalAuthorization() = 0;
  virtual void AttemptAuthorization() = 0;
  virtual void Logout() = 0;
  [[nodiscard]] virtual bool IsAuthorized() const = 0;
  [[nodiscard]] virtual QString GetLastErrorCode() const = 0;
  [[nodiscard]] virtual std::optional<QString> GetToken() const = 0;

signals:
  void authorized();
  void unauthorized();
  void logout();
  void logoutFinished();
  void logoutFailed(const QString& error);
  void authorizationFailed();
  void initializationFailed();
  void authorizationCanceled();
};
