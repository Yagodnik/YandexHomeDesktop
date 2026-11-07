#include <QDebug>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmap>
#include <QScreen>
#include <qt_windows.h>

#include <cstdio>

namespace {

QJsonArray Bounds(const QRect& rect) {
  return {rect.x(), rect.y(), rect.width(), rect.height()};
}

int Fail(const char* message) {
  qCritical("%s", message);
  return 1;
}

}  // namespace

int main(int argc, char** argv) {
  qputenv("QT_QPA_PLATFORM", "windows");
  QGuiApplication app(argc, argv);
  const auto args = app.arguments();
  const auto screens = app.screens();
  const auto command = args.value(1);
  if (command == "displays") {
    QJsonArray displays;
    for (const auto* screen : screens) {
      displays.append(QJsonObject{{"bounds", Bounds(screen->geometry())},
                                  {"work_area", Bounds(screen->availableGeometry())},
                                  {"scale", screen->devicePixelRatio()}});
    }
    const auto json = QJsonDocument(displays).toJson(QJsonDocument::Compact);
    std::puts(json.constData());
    return 0;
  }
  if (command == "click" && args.size() == 4) {
    bool x_ok = false;
    bool y_ok = false;
    const int x = args[2].toInt(&x_ok);
    const int y = args[3].toInt(&y_ok);
    if (!x_ok || !y_ok || !SetCursorPos(x, y))
      return Fail("Could not position the cursor on the interactive Windows desktop.");

    INPUT clicks[2]{};
    clicks[0].type = clicks[1].type = INPUT_MOUSE;
    clicks[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    clicks[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    if (SendInput(2, clicks, sizeof(INPUT)) != 2)
      return Fail("Could not send the native Windows mouse click.");
    return 0;
  }
  if (command == "capture" && !screens.empty() && args.size() == 2 + screens.size()) {
    for (qsizetype index = 0; index < screens.size(); ++index) {
      const auto image = screens[index]->grabWindow(0);
      if (image.isNull() || !image.save(args[index + 2], "PNG"))
        return Fail("Could not save a full Windows display screenshot.");
    }
    return 0;
  }
  return Fail("Expected displays, click x y, or capture followed by one PNG path per display.");
}
