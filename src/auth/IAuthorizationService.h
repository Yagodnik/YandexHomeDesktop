#pragma once

#include <QObject>
#include <QString>
#include <optional>

class IAuthorizationService : public QObject {
  Q_OBJECT
public:
  explicit IAuthorizationService(QObject *parent = nullptr) : QObject(parent) {}
  virtual ~IAuthorizationService() = default;

  Q_INVOKABLE virtual void TryLoadTokenFromStorage() = 0;
  Q_INVOKABLE virtual bool IsAuthorized() const = 0;
  Q_INVOKABLE virtual void AttemptAuthorization(const QVariant& user_data) = 0;
  Q_INVOKABLE virtual void SaveAuthToken(const QString &token) = 0;
  Q_INVOKABLE virtual void Logout() = 0;
  Q_INVOKABLE virtual QString GetLastErrorCode() const = 0;

  virtual std::optional<QString> GetToken() const = 0;

signals:
  void authorized();
  void unauthorized();

  void logout();
  void logoutFinished();
  void logoutFailed(const QString &error);

  void authorizationFailed();
  void initializationFailed();
  void authorizationCanceled();
};