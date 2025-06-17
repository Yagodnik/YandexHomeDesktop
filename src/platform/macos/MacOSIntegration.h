#pragma once

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace MacOSIntegration {
  void HideAppFromTheDock();

  void ShowAppFromTheDock();
}
