import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Pages
import YandexHomeDesktop.Ui

Window {
  id: window
  width: 350
  height: 460
  minimumWidth: 350
  minimumHeight: 460
  visible: !settings.trayModeEnabled
  title: qsTr("Yandex Home Desktop")
  color: "transparent"

  /* FPS Counter
  property int frames: 0
  property real fps: 0
  Timer {
    interval: 1000
    repeat: true
    running: true
    onTriggered: {
      fps = frames
      frames = 0
    }
  }

  onFrameSwapped: frames++
  Text {
    x: 10
    y: 10
    z: 3000
    color: "red"
    font.pixelSize: 20
    text: "FPS: " + fps
  } */

  Component.onCompleted: {
    platformService.SetWindow(window);
  }

  onActiveFocusItemChanged: {
    if (!activeFocusItem && settings.trayModeEnabled) {
      window.hide();
    }
  }

  onActiveChanged: {
    if (active) {
      deviceController.ContinuePollingIfNeeded();
    } else {
      deviceController.StopPolling();
    }
  }

  SystemTrayIcon {
    visible: settings.trayModeEnabled
    icon.name: "Yandex Home Desktop"
    icon.source: "qrc:/images/icon.png"

    onActivated: function(reason) {
      console.log("Position:", geometry.x, geometry.y);
      platformService.ShowWindow(geometry);
    }
  }

  QtObject {
    id: routerHelper

    function push(path) {
      pages.push(path);
    }

    function pop() {
      if (pages.depth > 1) {
        pages.pop()
      }
    }
  }

  Connections {
    target: authorizationService

    function onAuthorized() {
      router.navigateTo("main");
    }

    function onUnauthorized() {
      router.navigateTo("auth");
    }

    function onLogout() {
      router.navigateTo("auth");
    }

    function onAuthorizationFailed() {
      router.navigateTo("error");
    }

    function onInitializationFailed() {
      console.log("App is broken!");
      Qt.quit();
    }

    function onAuthorizationCanceled() {
      router.navigateTo("authCanceled");
    }
  }

  Rectangle {
    id: background
    anchors.fill: parent
    color: themes.background
    radius: settings.trayModeEnabled ? 12 : 0
  }

  StackView {
    id: pages
    anchors.fill: parent

    layer.enabled: settings.trayModeEnabled
    layer.smooth: true
    layer.effect: OpacityMask {
      maskSource: background
    }

    pushEnter: Transition {
      NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
    }

    pushExit: Transition {
      NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
    }

    popEnter: Transition {
      NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
    }

    popExit: Transition {
      NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
    }

    Component.onCompleted: {
      router.setView(routerHelper);

      router.addRoute("loading", "qrc:/pages/LoadingPage.qml");
      router.addRoute("auth", "qrc:/pages/AuthPage.qml");
      router.addRoute("main", "qrc:/pages/MainPage.qml");
      router.addRoute("error", "qrc:/pages/ErrorPage.qml");
      router.addRoute("device", "qrc:/pages/DevicePage.qml");
      router.addRoute("authCanceled", "qrc:/pages/AuthCanceledPage.qml");

      router.navigateTo("loading");
    }
  }
}
