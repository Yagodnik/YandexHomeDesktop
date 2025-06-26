import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import YandexHomeDesktop.Pages
import YandexHomeDesktop.Ui

Window {
  id: window
  width: 350
  height: 460
  visible: true
  title: qsTr("Yandex Home Desktop")
  // flags: Qt.FramelessWindowHint

  Rectangle {
    anchors.fill: parent
    color: themes.background
  }

  SystemTrayIcon {
    visible: true
    icon.name: "Yandex Home Desktop"
    icon.source: "qrc:/images/icon.png"

    onActivated: function(reason) {
      console.log("Position:", geometry.x, geometry.y);

      window.x = geometry.x - window.width / 2 + geometry.width / 2
      window.y = geometry.y + 10;

      window.show()
      window.raise()
      window.requestActivate()
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

  StackView {
    id: pages
    anchors.fill: parent

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
