#pragma once

#include "IAuthorizationService.h"

enum class AuthorizationMode { Interactive, SavedTokenOnly, Fixture };

// Application composition only. The core service and models use contracts.
IAuthorizationService* CreateAuthorizationService(AuthorizationMode mode, QObject* parent);
