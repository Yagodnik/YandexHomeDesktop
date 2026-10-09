#include "AppContext.h"
#include "auth/AuthorizationFactory.h"
#include <stdexcept>
#ifdef YH_DEBUG_FAKE_API
#include "api/debug/FixtureApi.h"
#endif

AppContext::AppContext(QGuiApplication *app, const StartupOptions& options)
  : app_(app), cli_arguments(options.cli_arguments.isEmpty() ? app->arguments() : options.cli_arguments) {
  authorization_service = CreateAuthorizationService(options.use_fake_api
    ? AuthorizationMode::Fixture : AuthorizationMode::Interactive, app_);
  platform_service = new PlatformService(app_);

  token_provider = [service = authorization_service] {
    return service->GetToken().value_or(QString{});
  };

  if (options.use_fake_api) {
#ifdef YH_DEBUG_FAKE_API
    auto* fixture = new FixtureApi(options.fixture_path, app_);
    const auto result = fixture->Validate();
    if (!result) {
      throw std::runtime_error(result.error().message.toStdString());
    }
    yandex_api = fixture;
    account_api = fixture;
#else
    throw std::runtime_error("The fake API is available only in Debug builds");
#endif
  } else {
    http_transport = new QtHttpTransport(app_);
    yandex_api = new YandexHomeApi(token_provider, http_transport, app_);
    account_api = new YandexAccountApi(token_provider, http_transport, app_);
  }
  yandex_account = new AccountModel(account_api, app_);
  home_service = new HomeService(yandex_api, app_);
  QObject::connect(authorization_service, &IAuthorizationService::logout,
    home_service, &HomeService::Reset);
  scenario_service = new ScenarioService(yandex_api, app_);
  device_service = new DeviceService(yandex_api, app_);
  QObject::connect(authorization_service, &IAuthorizationService::logout,
    scenario_service, &ScenarioService::Reset);
  QObject::connect(authorization_service, &IAuthorizationService::logout,
    yandex_account, &AccountModel::Reset);

  settings = new Settings(app_, options.use_fake_api);
}
