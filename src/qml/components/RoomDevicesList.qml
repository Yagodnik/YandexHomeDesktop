import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Column {
  id: room
  width: parent.width
  spacing: 8
  Text {
    id: roomTitle
    text: name
    color: themes.GetInactive()

    font.pointSize: 14
    font.bold: true
  }

  ListView {
    clip: true
    interactive: false
    spacing: 8

    width: parent.width
    height: contentHeight

    model: ListModel {
      ListElement {
        name: "Устройство"
      }

      ListElement {
        name: "Устройство"
      }
    }
    delegate: Components.DeviceDelegate {}
  }
}

