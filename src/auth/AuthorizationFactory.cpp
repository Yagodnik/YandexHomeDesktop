#include "AuthorizationFactory.h"

#include "AuthorizationService.h"
#include "KeychainTokenStore.h"
#include <stdexcept>
#ifdef YH_DEBUG_FAKE_API
#include "debug/FixtureAuthorizationService.h"
#endif

IAuthorizationService* CreateAuthorizationService(AuthorizationMode mode, QObject* parent, IAuthorizationFlow* flow) {
  if (mode == AuthorizationMode::Fixture) {
#ifdef YH_DEBUG_FAKE_API
    return new FixtureAuthorizationService(parent);
#else
    throw std::runtime_error("The fake API is available only in Debug builds");
#endif
  }
  auto* store = new KeychainTokenStore;
  auto* service = new AuthorizationService(store, flow, parent);
  store->setParent(service);
  if (auto* owner = dynamic_cast<QObject*>(flow)) { owner->setParent(service); }
  return service;
}
