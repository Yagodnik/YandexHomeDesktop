#include "PlatformServiceWin.h"
#include "WindowPlacement.h"

void PlatformServiceWin::ShowWindow(const QRect& icon) {
  if (!window_) return;
  window_->setPosition(WindowPlacement(icon, window_->size()));
  window_->show();
  window_->raise();
  window_->requestActivate();
}

void PlatformServiceWin::ShowAsApp() {
  if (window_ && window_->flags().testFlag(Qt::FramelessWindowHint)) {
    const bool visible = window_->isVisible();
    window_->setFlags(Qt::Window);
    if (visible) window_->show();
  }
}

void PlatformServiceWin::ShowOnlyInTray() {
  if (window_ && !window_->flags().testFlag(Qt::FramelessWindowHint)) {
    const bool visible = window_->isVisible();
    window_->setFlags(Qt::FramelessWindowHint);
    if (visible) window_->show();
  }
}
