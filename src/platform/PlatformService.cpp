#include "PlatformService.h"
#include "PlatformServiceFactory.h"
#include "NotchActivation.h"
#ifdef Q_OS_MACOS
#include "macos/MacOSIntegration.h"
#endif

PlatformService::PlatformService(QObject *parent)
  : QObject(parent), impl_(PlatformServiceFactory::Create()) {
#ifdef Q_OS_MACOS
  notch_timer_.setInterval(100);
  connect(&notch_timer_, &QTimer::timeout, this, [this] {
    const QRect notch = MacOSIntegration::NotchUnderCursor();
    const bool inside = notch.isValid();
    if (window_ && ShouldActivateFromNotch(inside, inside_notch_)) {
      if (window_->windowState() == Qt::WindowMinimized) window_->setWindowState(Qt::WindowNoState);
      ShowWindow(notch);
    }
    inside_notch_ = inside;
  });
#endif
}

bool PlatformService::NotchSupported() const {
#ifdef Q_OS_MACOS
  return true;
#else
  return false;
#endif
}

void PlatformService::SetNotchEnabled(bool enabled) {
  inside_notch_ = false;
  if (enabled && NotchSupported()) notch_timer_.start();
  else notch_timer_.stop();
}

void PlatformService::SetWindow(QQuickWindow *window) {
  window_ = window;
  impl_->SetWindow(window);
}

void PlatformService::ShowWindow(const QRect& icon) const {
  impl_->ShowWindow(icon);
}

void PlatformService::ShowAsApp() const {
  impl_->ShowAsApp();
}

void PlatformService::ShowOnlyInTray() const {
  impl_->ShowOnlyInTray();
}
