#pragma once

#include "PlatformServiceImpl.h"

class PlatformServiceMac final : public PlatformServiceImpl {
public:
  void ShowAsApp() override;
  void ShowOnlyInTray() override;
};
