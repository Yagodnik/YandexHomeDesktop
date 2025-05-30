import QtQuick
import Qt.labs.platform

Window {
  width: 320
  height: 380
  visible: true
  title: qsTr("Hello World")

  MyTabs {
    anchors.fill: parent

    tabs: [
      {
        title: "Устройства"
      },
      {
        title: "Сценарии"
      },
      {
        title: "Настройки"
      }
    ]
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
