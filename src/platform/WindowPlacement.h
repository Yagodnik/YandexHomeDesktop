#pragma once

#include <QGuiApplication>
#include <QScreen>
#include <QCursor>
#include <algorithm>

// Windows keeps the screen origin in native pixels when it scales Qt screen sizes.
inline QRect WindowsTrayAnchorInScreen(const QRect& native_anchor, const QRect& logical_screen,
                                       qreal scale) {
  const QPoint origin = logical_screen.topLeft();
  const QRect native_screen(origin, logical_screen.size() * scale);
  if (!native_anchor.isValid() || !native_screen.contains(native_anchor.center())) return {};

  const QPointF offset = QPointF(native_anchor.topLeft() - origin) / scale;
  const QSizeF size = QSizeF(native_anchor.size()) / scale;
  return QRectF(QPointF(origin) + offset, size).toRect();
}

// All coordinates are Qt logical coordinates, including on mixed-DPI desktops.
inline QPoint WindowPlacementInArea(const QRect& anchor, const QSize& size,
                                    const QRect& bounds, const QRect& area, const QPoint& fallback) {
  const QPoint center = anchor.isValid() ? anchor.center() : fallback;
  // Select the closest screen edge, including auto-hidden panels.
  const int left = qAbs(center.x() - bounds.left());
  const int right = qAbs(bounds.right() - center.x());
  const int top = qAbs(center.y() - bounds.top());
  const int bottom = qAbs(bounds.bottom() - center.y());
  int edge = std::min({left, right, top, bottom});
  if (center.y() < area.top()) edge = top;
  else if (center.y() > area.bottom()) edge = bottom;
  else if (center.x() < area.left()) edge = left;
  else if (center.x() > area.right()) edge = right;
  int x = center.x() - size.width() / 2;
  int y = center.y() - size.height() / 2;
  if (edge == top) y = anchor.isValid() ? anchor.bottom() + 6 : area.top();
  else if (edge == bottom) y = anchor.isValid() ? anchor.top() - size.height() - 6 : area.bottom() - size.height() + 1;
  else if (edge == left) x = anchor.isValid() ? anchor.right() + 6 : area.left();
  else x = anchor.isValid() ? anchor.left() - size.width() - 6 : area.right() - size.width() + 1;
  x = std::clamp(x, area.left(), std::max(area.left(), area.right() - size.width() + 1));
  y = std::clamp(y, area.top(), std::max(area.top(), area.bottom() - size.height() + 1));
  return {x, y};
}

inline QPoint WindowPlacement(const QRect& anchor, const QSize& size) {
  QScreen* screen = anchor.isValid() ? QGuiApplication::screenAt(anchor.center()) : nullptr;
  if (!screen) screen = QGuiApplication::screenAt(QCursor::pos());
  if (!screen) screen = QGuiApplication::primaryScreen();
  if (!screen) return {};
  return WindowPlacementInArea(anchor, size, screen->geometry(), screen->availableGeometry(), QCursor::pos());
}
