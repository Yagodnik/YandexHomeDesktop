#include "AuthorizationFactory.h"

#include "AuthorizationService.h"
#include "KeychainTokenStore.h"
#include "QtOAuthAuthorizationFlow.h"
#include <stdexcept>
#ifdef YH_DEBUG_FAKE_API
#include "debug/FixtureAuthorizationService.h"
#endif

IAuthorizationService* CreateAuthorizationService(AuthorizationMode mode, QObject* parent) {
  if (mode == AuthorizationMode::Fixture) {
#ifdef YH_DEBUG_FAKE_API
    return new FixtureAuthorizationService(parent);
#else
    throw std::runtime_error("The fake API is available only in Debug builds");
#endif
  }
  auto* store = new KeychainTokenStore;
  auto* flow = mode == AuthorizationMode::Interactive ? new QtOAuthAuthorizationFlow : nullptr;
  auto* service = new AuthorizationService(store, flow, parent);
  store->setParent(service);
  if (flow) { flow->setParent(service); }
  return service;
}
