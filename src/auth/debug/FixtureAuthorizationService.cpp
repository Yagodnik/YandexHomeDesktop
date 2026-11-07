#include "FixtureAuthorizationService.h"

#include <QTimer>

void FixtureAuthorizationService::AttemptLocalAuthorization() {
  if (logout_requested_) { return; }
  const auto generation = ++generation_;
  QTimer::singleShot(0, this, [this, generation] {
    if (generation != generation_) { return; }
    if (authorized_) { emit authorized(); } else { emit unauthorized(); }
  });
}

void FixtureAuthorizationService::AttemptAuthorization() {
  if (logout_requested_) { return; }
  authorized_ = true;
  AttemptLocalAuthorization();
}

void FixtureAuthorizationService::Logout() {
  if (logout_requested_) { return; }
  ++generation_;
  logout_requested_ = true;
  authorized_ = false;
  emit logout();
  QTimer::singleShot(0, this, [this] {
    logout_requested_ = false;
    emit logoutFinished();
  });
}
