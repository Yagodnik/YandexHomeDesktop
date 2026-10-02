#include "AppContext.h"

AppContext::AppContext(QGuiApplication *app): app_(app) {
  authorization_service = new AuthorizationService(app_);
  platform_service = new PlatformService(app_);

  token_provider = [this] -> QString {
    const auto token = authorization_service->GetToken();
    if (!token.has_value()) {
      qWarning() << "AuthorizationService::GetToken: no token provided";
      QGuiApplication::exit(0);
      return "";
    }

    return token.value();
  };

  http_transport = new QtHttpTransport(app_);
  yandex_api = new YandexHomeApi(token_provider, http_transport, app_);
  account_api = new YandexAccountApi(token_provider, http_transport, app_);
  yandex_account = new AccountModel(account_api, app_);
  home_snapshot_loader = new HomeSnapshotLoader(yandex_api, app_);

  settings = new Settings(app_);
}
