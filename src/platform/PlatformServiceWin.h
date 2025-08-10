#pragma once

#include "PlatformServiceImpl.h"
#include <windows.h>

class PlatformServiceWin final : public PlatformServiceImpl {
public:
  void ShowWindow(const QRect& icon) override;
  void ShowAsApp() override;
  void ShowOnlyInTray() override;

private:
  RECT GetTaskbarRectUnderCursor();
};
