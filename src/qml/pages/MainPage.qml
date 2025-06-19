import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.platform
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Pages as Pages
import YandexHomeDesktop.Components as Components

Item {
  Rectangle {
    id: backdrop
    anchors.fill: parent
    color: "#80000000"
    visible: householdSelectDialog.myVisible
    enabled: householdSelectDialog.myVisible
    z: 500

    MouseArea {
      anchors.fill: parent
      onClicked: householdSelectDialog.myVisible = false
      enabled: true
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

    Rectangle {
      anchors.fill: parent
      color: themes.GetBackground()
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