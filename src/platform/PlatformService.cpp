#include "PlatformService.h"
#include "PlatformServiceFactory.h"

PlatformService::PlatformService(QObject *parent)
  : QObject(parent), impl_(PlatformServiceFactory::Create()) {}

void PlatformService::SetWindow(QQuickWindow *window) const {
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
