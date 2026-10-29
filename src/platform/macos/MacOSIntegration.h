#pragma once
#include <QRect>

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace MacOSIntegration {
  QRect NotchUnderCursor();
  void HideAppFromTheDock();

  void ShowAppFromTheDock();
}
