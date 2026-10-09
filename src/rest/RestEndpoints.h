#pragma once

#include "yh/apprest_export.h"

#include "RestRouter.h"

class HomeService;
class DeviceService;
class ScenarioService;
class AccountService;

APPREST_EXPORT RestRouter CreateRestRoutes(
  HomeService& home, DeviceService& devices, ScenarioService& scenarios, AccountService& account);
