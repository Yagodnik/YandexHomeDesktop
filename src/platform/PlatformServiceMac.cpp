#include "PlatformServiceMac.h"
#include "WindowPlacement.h"
#include "macos/MacOSIntegration.h"

void PlatformServiceMac::ShowWindow(const QRect& icon) {
  if (!window_) return;
  window_->setPosition(WindowPlacement(icon, window_->size()));
  window_->show();
  window_->raise();
  window_->requestActivate();
}

void PlatformServiceMac::ShowAsApp() {
  MacOSIntegration::ShowAppFromTheDock();

  if (window_ && window_->flags().testFlag(Qt::FramelessWindowHint)) {
    const bool visible = window_->isVisible();
    window_->setFlags(Qt::Window);
    if (visible) window_->show();
  }
}

void PlatformServiceMac::ShowOnlyInTray() {
  MacOSIntegration::HideAppFromTheDock();

  if (window_ && !window_->flags().testFlag(Qt::FramelessWindowHint)) {
    const bool visible = window_->isVisible();
    window_->setFlags(Qt::FramelessWindowHint);
    if (visible) window_->show();
  }
}
