#pragma once

#include <QPointF>
#include <QRectF>

// The physical cutout can be unreachable by the pointer. Include a narrow strip
// directly below it, while keeping the cutout itself as the window anchor.
inline bool IsNotchActivationPoint(const QRectF& notch, const QPointF& point) {
  return notch.isValid() && notch.adjusted(0, 0, 0, 12).contains(point);
}

// Activate once per entry, even if the window manager reports an obscured
// window as active. Leaving the target rearms the trigger.
inline bool ShouldActivateFromNotch(bool inside, bool wasInside) {
  return inside && !wasInside;
}
