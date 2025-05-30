import QtQuick
import Qt.labs.platform

Window {
  width: 640
  height: 480
  visible: true
  title: qsTr("Hello World")

  Text {
    text: "Hello, world"
    anchors.centerIn: parent
  }

  SystemTrayIcon {
    visible: true
    icon.source: "qrc:/images/icon.png"

    menu: Menu {
      MenuItem {
        text: qsTr("Quit")
        onTriggered: Qt.quit()
      }
    }
  }
}
