#pragma once

#include <QObject>

#include "auth/AuthorizationService.h"
#include "platform/PlatformService.h"
#include "utils/Settings.h"
#include "api/YandexAccount.h"
#include "api/YandexHomeApi.h"

struct AppContext {
  explicit AppContext(QGuiApplication *app);

  QGuiApplication *app_;
  AuthorizationService *authorization_service;
  PlatformService *platform_service;
  YandexHomeApi *yandex_api;
  YandexAccount *yandex_account;
  Settings *settings;

  std::function<QString()> token_provider;
};
