#pragma once

#include "platform/PlatformService.h"
#include "utils/Settings.h"
#include "api/YandexAccount.h"
#include "api/YandexHomeApi.h"
#include "auth/WebAuthorizationService.h"

struct AppContext {
  explicit AppContext(QGuiApplication *app);

  QGuiApplication *app_;
  IAuthorizationService *authorization_service;
  PlatformService *platform_service;
  YandexHomeApi *yandex_api;
  YandexAccount *yandex_account;
  Settings *settings;

  std::function<QString()> token_provider;
};
