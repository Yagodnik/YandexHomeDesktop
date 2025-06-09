import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.platform
import "./ui/" as UI
import "./components" as Components
import "./pages/" as Pages

Window {
  width: 350
  height: 460
  visible: true
  title: qsTr("Yandex Home Desktop")
  // flags: Qt.FramelessWindowHint

  StackLayout {
    id: pages
    anchors.fill: parent

    currentIndex: 0

    Pages.MainPage {}

    Pages.LoadingPage {}

    Pages.AuthPage {}
  }
}
