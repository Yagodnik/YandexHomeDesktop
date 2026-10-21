#pragma once

#include "yh/yandexauth_export.h"

#include "IAuthorizationService.h"
#include "IAuthorizationFlow.h"

enum class AuthorizationMode { Interactive, SavedTokenOnly, Fixture };

// Application composition only. The core service and models use contracts.
YANDEXAUTH_EXPORT IAuthorizationService* CreateAuthorizationService(AuthorizationMode mode, QObject* parent, IAuthorizationFlow* flow = nullptr);
