#include "PlatformServiceMac.h"
#include "macos/MacOSIntegration.h"

void PlatformServiceMac::ShowAsApp() {
  MacOSIntegration::ShowAppFromTheDock();
}

void PlatformServiceMac::ShowOnlyInTray() {
  MacOSIntegration::HideAppFromTheDock();
}
