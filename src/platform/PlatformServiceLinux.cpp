#include "PlatformServiceLinux.h"
#include "WindowPlacement.h"

void PlatformServiceLinux::ShowWindow(const QRect& icon) {
  if (!window_) return;
  window_->setPosition(WindowPlacement(icon, window_->size()));
  window_->show();
  window_->raise();
  window_->requestActivate();
}

void PlatformServiceLinux::ShowAsApp() {
  if (window_ && window_->flags().testFlag(Qt::FramelessWindowHint)) {
    const bool visible = window_->isVisible();
    window_->setFlags(Qt::Window);
    if (visible) window_->show();
  }
}

void PlatformServiceLinux::ShowOnlyInTray() {
  if (window_ && !window_->flags().testFlag(Qt::FramelessWindowHint)) {
    const bool visible = window_->isVisible();
    window_->setFlags(Qt::FramelessWindowHint);
    if (visible) window_->show();
  }
}
