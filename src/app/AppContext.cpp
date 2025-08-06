#include "AppContext.h"

AppContext::AppContext(QGuiApplication *app): app_(app) {
  authorization_service = new AuthorizationService(app_);
  platform_service = new PlatformService(app_);

  token_provider = [this] {
    const auto token = authorization_service->GetToken();
    if (!token.has_value()) {
      qWarning() << "AuthorizationService::GetToken: no token provided";
      QGuiApplication::quit();
    }

    return token.value();
  };

  yandex_api = new YandexHomeApi(token_provider, app_);
  yandex_account = new YandexAccount(token_provider, app_);

  settings = new Settings(app_);
}
