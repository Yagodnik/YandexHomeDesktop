#include "PlatformServiceWin.h"
#include <shellapi.h>

void PlatformServiceWin::ShowWindow(const QRect &icon) {
  if (window_ == nullptr) {
    qCritical() << "PlatformServiceWin: Nullptr window";
    return;
  }

  RECT tb = GetTaskbarRectUnderCursor();
  const int taskbar_width = tb.right - tb.left;
  const int taskbar_height = tb.bottom - tb.top;

  APPBARDATA abd{};
  abd.cbSize = sizeof(abd);

  QPoint position;
  if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
    switch (abd.uEdge) {
      case ABE_LEFT: {
        position.setX(icon.x() + taskbar_width + 5);
        position.setY(icon.y() - window_->height() / 2 - icon.height());
        break;
      } case ABE_TOP: {
        position.setX(icon.x() - window_->width() / 2 + icon.width());
        position.setY(icon.y() + 15 + taskbar_height);
        break;
      } case ABE_RIGHT: {
        position.setX(icon.x() - window_->width() - 15);
        position.setY(icon.y() - window_->height() / 2 - icon.height());
        break;
      } case ABE_BOTTOM: {
        position.setX(icon.x() - window_->width() / 2 + icon.width() / 2);
        position.setY(icon.y() - window_->height() - 5);
        break;
      }
    }
  } else {
    qCritical() << "PlatformServiceWin: Unable to get taskbar rect";
  }

  window_->setPosition(position);

  window_->show();
  window_->raise();
  window_->requestActivate();
}

void PlatformServiceWin::ShowAsApp() {
  if (window_ != nullptr) {
    window_->setFlags(Qt::Window);
  }
}

void PlatformServiceWin::ShowOnlyInTray() {
  if (window_ != nullptr) {
    window_->setFlags(Qt::FramelessWindowHint);
  }
}

RECT PlatformServiceWin::GetTaskbarRectUnderCursor() {
  POINT cursorPos;
  GetCursorPos(&cursorPos);

  HMONITOR monitor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);

  MONITORINFO mi{};
  mi.cbSize = sizeof(mi);
  if (!GetMonitorInfo(monitor, &mi)) {
    return {0, 0, 0, 0};
  }

  RECT monitorRect = mi.rcMonitor;
  RECT workRect    = mi.rcWork;
  RECT taskbarRect = {0, 0, 0, 0};

  if (workRect.top > monitorRect.top) {
    // TOP
    taskbarRect = { monitorRect.left, monitorRect.top,
                    monitorRect.right, workRect.top };
  }
  else if (workRect.bottom < monitorRect.bottom) {
    // BOTTOM
    taskbarRect = { monitorRect.left, workRect.bottom,
                    monitorRect.right, monitorRect.bottom };
  }
  else if (workRect.left > monitorRect.left) {
    // LEFT
    taskbarRect = { monitorRect.left, monitorRect.top,
                    workRect.left, monitorRect.bottom };
  }
  else if (workRect.right < monitorRect.right) {
    // RIGHT
    taskbarRect = { workRect.right, monitorRect.top,
                    monitorRect.right, monitorRect.bottom };
  }

  return taskbarRect;
}