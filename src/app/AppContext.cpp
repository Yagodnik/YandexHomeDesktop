#include "AppContext.h"

#include "auth/YandexTokenAuthorizationService.h"

AppContext::AppContext(QGuiApplication *app): app_(app) {
  // authorization_service = new WebAuthorizationService(app_);

  network_manager_ = new QNetworkAccessManager(app_);
  authorization_service = new YandexTokenAuthorizationService(network_manager_, app_);

  platform_service = new PlatformService(app_);

  token_provider = [this] -> QString {
    const auto token = authorization_service->GetToken();
    if (!token.has_value()) {
      qWarning() << "AuthorizationService::GetToken: no token provided";
      // QGuiApplication::exit(0);
      return "";
    }

    return token.value();
  };

  yandex_api = new YandexHomeApi(network_manager_, token_provider, app_);
  yandex_account = new YandexAccount(network_manager_, token_provider, app_);

  settings = new Settings(app_);
}
