#pragma once

#include <QObject>

#include "../auth/AuthorizationService.h"
#include "../platform/PlatformService.h"
#include "../utils/Settings.h"
#include "../api/QtHttpTransport.h"
#include "../api/YandexAccountApi.h"
#include "../api/YandexHomeApi.h"
#include "../models/AccountModel.h"
#include "../models/HomeSnapshotLoader.h"

struct AppContext {
  explicit AppContext(QGuiApplication *app);

  QGuiApplication *app_;
  AuthorizationService *authorization_service;
  PlatformService *platform_service;
  QtHttpTransport *http_transport;
  YandexHomeApi *yandex_api;
  YandexAccountApi *account_api;
  AccountModel *yandex_account;
  HomeSnapshotLoader *home_snapshot_loader;
  Settings *settings;

  std::function<QString()> token_provider;
};
