import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import YandexHomeDesktop.Pages
import YandexHomeDesktop.Ui

Window {
  width: 350
  height: 460
  visible: true
  title: qsTr("Yandex Home Desktop")
  // flags: Qt.FramelessWindowHint

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

      router.navigateTo("loading");
    }
  }
}
