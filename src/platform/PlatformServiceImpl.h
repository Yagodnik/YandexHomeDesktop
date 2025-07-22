#pragma once

#include <QQuickWindow>

class PlatformServiceImpl {
public:
  virtual ~PlatformServiceImpl() = default;

  void SetWindow(QQuickWindow *window) {
    window_ = window;
  }

  virtual void ShowWindow(const QRect& icon) = 0;
  virtual void ShowAsApp() = 0;
  virtual void ShowOnlyInTray() = 0;

protected:
  QQuickWindow* window_ = nullptr;
};
