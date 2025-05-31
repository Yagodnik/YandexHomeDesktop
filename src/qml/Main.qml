import QtQuick
import QtQuick.Layouts
import Qt.labs.platform
import "./components/" as Components

Window {
  width: 350
  height: 460
  visible: true
  title: qsTr("Yandex Home Desktop")
  // flags: Qt.FramelessWindowHint

  Rectangle {
    id: backdrop
    anchors.fill: parent
    color: "#80000000"
    visible: householdSelectDialog.visible
    z: 1

    MouseArea {
      anchors.fill: parent
      onClicked: householdSelectDialog.visible = false
    }
  }

  Components.SelectDialog {
    id: householdSelectDialog

    title: "Выберите Дом"

    model: [
      { text: "Мой дом" },
      { text: "Дача" },
      { text: "Сарай" }
    ]

    delegate: Item {
      width: parent.width
      height: 48

      Text {
        id: houseName
        text: modelData.text
        color: Qt.rgba(0, 0, 0, 0.8);

        font.pointSize: 16
      }

      Text {
        id: houseAddress
        anchors.top: houseName.bottom
        anchors.topMargin: 4

        text: "Адрес не указан"
        color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1);

        font.pointSize: 13
      }
    }
  }

  Item {
    id: appRoot

    anchors.fill: parent
    anchors.top: parent.top
    anchors.topMargin: 20

    Rectangle {
      anchors.fill: parent
      color: Qt.rgba(242 / 255, 243 / 255, 245 / 255, 1)
    }

    Item {
      id: topBar

      width: parent.width
      height: layout.implicitHeight

      Rectangle {
        anchors.fill: parent
        color: "#FFF"
      }

      Column {
        id: layout
        anchors.fill: parent
        spacing: 4

        Item {
          id: householdSelect
          height: 28
          width: parent.width

          anchors.left: parent.left
          anchors.leftMargin: 16

          Row {
            Text {
              text: "Мой дом"
              font.pointSize: 24
              font.bold: true

              MouseArea {
                anchors.fill: parent

                onClicked: {
                  householdSelectDialog.visible = true
                }
              }
            }
          }
        }

        Row {
          id: tabs
          spacing: 16

          anchors.left: parent.left
          anchors.leftMargin: 16
          width: parent.width

          property int activeTab: 0

          MyTab {
            text: "Устройства"
            leftCorner: true
            active: tabs.activeTab === 0
            onClicked: tabs.activeTab = 0
          }

          MyTab {
            text: "Сценарии"
            leftCorner: true
            active: tabs.activeTab === 1
            onClicked: tabs.activeTab = 1
          }

          MyTab {
            text: "Настройки"
            leftCorner: true
            active: tabs.activeTab === 2
            onClicked: tabs.activeTab = 2
          }
        }
      }
    }

    Item {
      id: rooms

      anchors.top: topBar.bottom
      anchors.topMargin: 20
      anchors.left: parent.left
      anchors.leftMargin: 16

      Text {
        id: roomsText
        text: "Комнаты"
        color: Qt.rgba(0, 0, 0, 0.8)
        font.pointSize: 16
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
  }
}
