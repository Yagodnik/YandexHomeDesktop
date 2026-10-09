#pragma once

#include <QObject>

#include "auth/IAuthorizationService.h"
#include "../platform/PlatformService.h"
#include "../utils/Settings.h"
#include "../api/QtHttpTransport.h"
#include "../api/YandexAccountApi.h"
#include "../api/YandexHomeApi.h"
#include "../models/AccountModel.h"
#include "services/HomeService.h"
#include "services/ScenarioService.h"
#include "services/DeviceService.h"
#include "StartupOptions.h"

struct AppContext {
  explicit AppContext(QGuiApplication *app, const StartupOptions& options = {});

  QGuiApplication *app_;
  IAuthorizationService *authorization_service;
  PlatformService *platform_service;
  QtHttpTransport *http_transport = nullptr;
  IHomeApi *yandex_api;
  IAccountApi *account_api;
  AccountModel *yandex_account;
  HomeService *home_service;
  ScenarioService *scenario_service;
  DeviceService *device_service;
  Settings *settings;
  QStringList cli_arguments;

  std::function<QString()> token_provider;
};
