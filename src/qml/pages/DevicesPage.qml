import QtQuick
import "../ui" as UI

Item {
  id: rooms

  UI.DefaultText {
    id: roomsText
    text: "Комнаты"


    font.bold: true
  }

  Column {
    anchors.top: roomsText.bottom
    anchors.topMargin: 2

    Item {
      id: room

      Text {
        id: roomTitle
        text: "Лампа"
        color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1)

        font.pointSize: 14
        font.bold: true
      }

      Column {
        anchors.top: roomTitle.bottom
        anchors.topMargin: 8

        Item {
          id: deviceDelegate
          width: parent.width
          height: 64

          Rectangle {
            anchors.fill: parent
            color: "red"
            radius: 16
          }

          Text {
            text: "Лампочка"
          }
        }
      }
    }
  }
}
