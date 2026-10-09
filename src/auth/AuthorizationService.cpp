#include "AuthorizationService.h"

#include <QDebug>

AuthorizationService::AuthorizationService(ITokenStore* store, IAuthorizationFlow* flow,
                                           QObject* parent)
  : IAuthorizationService(parent), store_(store), flow_(flow) {
  Q_ASSERT(store_);
}

void AuthorizationService::AttemptLocalAuthorization() {
  if (operation_ != Operation::Idle || logout_requested_) { return; }
  const auto generation = ++generation_;
  token_.reset();
  last_error_code_ = 0;
  operation_ = Operation::Reading;
  store_->Read(this, [this, generation](AuthResult<QString> result) {
    if (generation != generation_ || operation_ != Operation::Reading) { return; }
    operation_ = Operation::Idle;
    if (!result) {
      Fail(result.error());
    } else if (result->isEmpty()) {
      emit unauthorized();
    } else {
      token_ = std::move(*result);
      emit authorized();
    }
  });
}

void AuthorizationService::AttemptAuthorization() {
  if (logout_requested_ || operation_ == Operation::Authorizing ||
      operation_ == Operation::Writing) { return; }
  const auto generation = ++generation_;
  token_.reset();
  last_error_code_ = 0;
  operation_ = Operation::Authorizing;
  if (!flow_) {
    operation_ = Operation::Idle;
    Fail({AuthErrorKind::Authorization, 0x80000002u, {}});
    return;
  }
  flow_->Start(this, [this, generation](AuthResult<QString> result) {
    if (generation != generation_ || operation_ != Operation::Authorizing) { return; }
    operation_ = Operation::Idle;
    if (!result) {
      Fail(result.error());
      return;
    }
    if (result->isEmpty()) {
      Fail({AuthErrorKind::Authorization, 0x80000003u, {}});
      return;
    }
    token_ = std::move(*result);
    operation_ = Operation::Writing;
    store_->Write(*token_, this, [this](AuthResult<void> saved) {
      if (operation_ != Operation::Writing) { return; }
      operation_ = Operation::Idle;
      if (logout_requested_) {
        // Delete after the pending write so it cannot recreate credentials.
        DeleteToken();
      } else if (!saved) {
        last_error_code_ = saved.error().code;
        qWarning() << "AuthorizationService: Could not persist the session";
      }
    });
    if (generation == generation_ && !logout_requested_) { emit authorized(); }
  });
}

void AuthorizationService::Logout() {
  if (logout_requested_) { return; }
  ++generation_;
  token_.reset();
  last_error_code_ = 0;
  logout_requested_ = true;
  if (flow_) { flow_->Cancel(); }
  emit logout();
  if (logout_requested_ && operation_ != Operation::Writing && operation_ != Operation::Deleting) {
    DeleteToken();
  }
}

void AuthorizationService::DeleteToken() {
  operation_ = Operation::Deleting;
  store_->Delete(this, [this](AuthResult<void> result) {
    if (operation_ != Operation::Deleting) { return; }
    operation_ = Operation::Idle;
    logout_requested_ = false;
    if (!result && result.error().kind != AuthErrorKind::NotFound) {
      last_error_code_ = result.error().code;
      emit logoutFailed(result.error().message);
    } else {
      emit logoutFinished();
    }
  });
}

bool AuthorizationService::IsAuthorized() const { return token_.has_value(); }
std::optional<QString> AuthorizationService::GetToken() const { return token_; }
QString AuthorizationService::GetLastErrorCode() const {
  return QString::number(last_error_code_, 16);
}

void AuthorizationService::Fail(const AuthError& error) {
  last_error_code_ = error.code;
  switch (error.kind) {
    case AuthErrorKind::NotFound: emit unauthorized(); break;
    case AuthErrorKind::Canceled: emit authorizationCanceled(); break;
    case AuthErrorKind::Initialization: emit initializationFailed(); break;
    default: emit authorizationFailed(); break;
  }
}
