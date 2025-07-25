#pragma once

#include "PlatformServiceImpl.h"

class PlatformServiceLinux final : public PlatformServiceImpl {
public:
  void ShowWindow(const QRect& icon) override;
  void ShowAsApp() override;
  void ShowOnlyInTray() override;
};