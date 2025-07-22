#include "PlatformServiceMac.h"
#include "macos/MacOSIntegration.h"

void PlatformServiceMac::ShowWindow(const QRect& icon) {
  if (window_ == nullptr) {
    qCritical() << "PlatformServiceMac: Nullptr window";
    return;
  }

  window_->setPosition(
    icon.x() - window_->width() / 2 + icon.width() / 2,
    icon.y() + 50
  );

  window_->show();
  window_->raise();
  window_->requestActivate();
}

void PlatformServiceMac::ShowAsApp() {
  MacOSIntegration::ShowAppFromTheDock();

  if (window_ != nullptr) {
    window_->setFlags(Qt::Window);
  }
}

void PlatformServiceMac::ShowOnlyInTray() {
  MacOSIntegration::HideAppFromTheDock();

  if (window_ != nullptr) {
    window_->setFlags(Qt::FramelessWindowHint);
  }
}
