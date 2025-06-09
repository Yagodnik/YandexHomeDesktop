import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.platform
import "../ui/" as UI
import "../components" as Components
import "./" as Pages

Item {
  Rectangle {
    id: backdrop
    anchors.fill: parent
    color: "#80000000"
    visible: householdSelectDialog.myVisible
    z: 1

    MouseArea {
      anchors.fill: parent
      onClicked: householdSelectDialog.myVisible = false
    }
  }

  UI.SelectDialog {
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

      UI.DefaultText {
        id: houseName
        text: modelData.text

      }

      UI.SubheadingText {
        id: houseAddress

        anchors.top: houseName.bottom
        anchors.topMargin: 4

        text: "Адрес не указан"
      }
    }
  }

  Item {
    id: appRoot

    anchors.fill: parent
    anchors.topMargin: 20

    Rectangle {
      anchors.fill: parent
      color: Qt.rgba(242 / 255, 243 / 255, 245 / 255, 1)
    }

    Components.TopBar {
      id: topBar
    }

    Item {
      clip: true
      anchors.top: topBar.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      StackLayout {
        anchors.fill: parent
        anchors.topMargin: 16
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.bottomMargin: 16

        currentIndex: topBar.activeTab

        Pages.DevicesPage {}

        Pages.ScenariosPage {}

        Pages.SettingsPage {}
      }
    }
  }
}