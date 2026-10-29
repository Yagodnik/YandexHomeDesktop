#pragma once
#include <QRect>

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace MacOSIntegration {
  bool HasNotch();
  QRect NotchUnderCursor();
  void HideAppFromTheDock();

  void ShowAppFromTheDock();
}
