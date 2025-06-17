#include "PlatformService.h"
#include "PlatformServiceFactory.h"

PlatformService::PlatformService(QObject *parent)
  : QObject(parent), impl_(PlatformServiceFactory::Create()) {}

void PlatformService::ShowAsApp() const {
  impl_->ShowAsApp();
}

void PlatformService::ShowOnlyInTray() const {
  impl_->ShowOnlyInTray();
}
