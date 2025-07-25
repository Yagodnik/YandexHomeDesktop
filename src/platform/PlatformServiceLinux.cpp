#include "PlatformServiceLinux.h"

void PlatformServiceLinux::ShowWindow(const QRect &icon) {
  if (window_ == nullptr) {
    qCritical() << "PlatformServiceLinux: Nullptr window";
    return;
  }

  window_->show();
  window_->raise();
  window_->requestActivate();
}

void PlatformServiceLinux::ShowAsApp() {
  if (window_ != nullptr) {
    window_->setFlags(Qt::Window);
  }
}

void PlatformServiceLinux::ShowOnlyInTray() {
  if (window_ != nullptr) {
    window_->setFlags(Qt::FramelessWindowHint);
  }
}
