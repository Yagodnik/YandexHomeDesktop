#include "PlatformServiceWin.h"
#include "WindowPlacement.h"

void PlatformServiceWin::ShowWindow(const QRect& icon) {
  if (!window_) return;
  // Qt Labs exposes the native Windows shell rectangle.
  QRect logical_icon;
  for (const auto* screen : QGuiApplication::screens()) {
    logical_icon = WindowsTrayAnchorInScreen(icon, screen->geometry(), screen->devicePixelRatio());
    if (logical_icon.isValid()) break;
  }
  window_->setPosition(WindowPlacement(logical_icon, window_->size()));
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
