#include "AuthorizationModel.h"

AuthorizationModel::AuthorizationModel(IAuthorizationService* service, QObject* parent)
  : QObject(parent), service_(service) {
  connect(service_, &IAuthorizationService::authorized, this, &AuthorizationModel::authorized);
  connect(service_, &IAuthorizationService::unauthorized, this, &AuthorizationModel::unauthorized);
  connect(service_, &IAuthorizationService::logout, this, &AuthorizationModel::logout);
  connect(service_, &IAuthorizationService::logoutFinished, this, &AuthorizationModel::logoutFinished);
  connect(service_, &IAuthorizationService::logoutFailed, this, &AuthorizationModel::logoutFailed);
  connect(service_, &IAuthorizationService::authorizationFailed, this, &AuthorizationModel::authorizationFailed);
  connect(service_, &IAuthorizationService::initializationFailed, this, &AuthorizationModel::initializationFailed);
  connect(service_, &IAuthorizationService::authorizationCanceled, this, &AuthorizationModel::authorizationCanceled);
}

void AuthorizationModel::AttemptLocalAuthorization() { service_->AttemptLocalAuthorization(); }
void AuthorizationModel::AttemptAuthorization() { service_->AttemptAuthorization(); }
void AuthorizationModel::Logout() { service_->Logout(); }
bool AuthorizationModel::IsAuthorized() const { return service_->IsAuthorized(); }
QString AuthorizationModel::GetLastErrorCode() const { return service_->GetLastErrorCode(); }
