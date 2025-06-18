import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: topBar

  width: parent.width
  height: layout.implicitHeight

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
  }

  property int activeTab: 0

  Column {
    id: layout
    anchors.fill: parent
    spacing: 12

    Item {
      id: householdSelect
      height: 28
      width: parent.width

      anchors.left: parent.left
      anchors.leftMargin: 16

      UI.HeadingText {
        text: "Мой дом"
        font.pointSize: 24
        font.bold: true
        anchors.top: parent.top
        anchors.topMargin: 20

        MouseArea {
          anchors.fill: parent

          onClicked: {
            householdSelectDialog.myVisible = true
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

      UI.MyTab {
        text: "Устройства"
        leftCorner: true
        active: topBar.activeTab === 0
        onClicked: topBar.activeTab = 0
      }

      UI.MyTab {
        text: "Сценарии"
        leftCorner: true
        active: topBar.activeTab === 1
        onClicked: topBar.activeTab = 1
      }

      UI.MyTab {
        text: "Настройки"
        leftCorner: true
        active: topBar.activeTab === 2
        onClicked: topBar.activeTab = 2
      }
    }
  }
}
