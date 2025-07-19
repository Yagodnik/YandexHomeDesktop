#include "PlatformServiceWin.h"

void PlatformServiceWin::ShowWindow(const QRect &icon) {
  if (window_ == nullptr) {
    qCritical() << "PlatformServiceWin: Nullptr window";
    return;
  }

  // TODO: Add taskbar position here
  window_->setPosition(
    icon.x(),
    icon.y() - window_->height() - 50
  );

  window_->show();
  window_->raise();
  window_->requestActivate();
}

void PlatformServiceWin::ShowAsApp() {
  if (window_ != nullptr) {
    window_->setFlags(Qt::Window);
  }
}

void PlatformServiceWin::ShowOnlyInTray() {
  if (window_ != nullptr) {
    window_->setFlags(Qt::FramelessWindowHint);
  }
}
