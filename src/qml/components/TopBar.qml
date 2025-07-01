import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  id: topBar

  width: parent.width
  height: layout.implicitHeight

  Rectangle {
    id: background
    anchors.fill: parent
    color: themes.headerBackground
  }

  property int activeTab: 0

  Column {
    id: layout
    anchors.fill: parent
    spacing: 12

    Item {
      id: householdSelect
      width: parent.width
      height: 28

      anchors.left: parent.left
      anchors.leftMargin: 16

      Item {
        width: heading.width + 4 + headingIcon.width
        height: 28

        anchors.top: parent.top
        anchors.topMargin: 16

        UI.HeadingText {
          id: heading
          text: "Мой дом"
          font.pointSize: 24
          font.bold: true
          anchors.left: parent.left
          anchors.verticalCenter: parent.verticalCenter
        }

        Image {
          id: headingIcon
          source: "qrc:/images/arrow.svg"

          anchors.left: heading.right
          anchors.leftMargin: 2

          anchors.verticalCenter: parent.verticalCenter

          width: 18
          height: 18
          rotation: 180
        }

        ColorOverlay {
          anchors.fill: headingIcon
          source: headingIcon
          color: themes.mainText
          rotation: 180
        }

        MouseArea {
          anchors.fill: parent

          onClicked: {
            householdSelectDialog.open();
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
