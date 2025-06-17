#pragma once

class PlatformServiceImpl {
public:
  virtual ~PlatformServiceImpl() = default;

  virtual void ShowAsApp() = 0;
  virtual void ShowOnlyInTray() = 0;
};
