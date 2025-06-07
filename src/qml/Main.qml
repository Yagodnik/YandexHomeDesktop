import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.platform
import "./ui/" as UI
import "./components" as Components
import "./pages/" as Pages
import Yandex 1.0

Window {
  width: 350
  height: 460
  visible: true
  title: qsTr("Yandex Home Desktop")
  // flags: Qt.FramelessWindowHint

  YandexAuth {
    id: authorization

    onAuthorized: {
      console.log("Auth: Ok!");
    }

    onAuthorizationFailed: {
      console.log("Auth: Failed!");
    }

    onInitializationFailed: {
      console.log("Auth: Init Failed!");
      Qt.quit();
    }
  }

  Button {
    text: "Auth!"
    x: 100
    y: 100
    z: 200

    onClicked: authorization.AttemptAuthorization();
  }

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
    anchors.top: parent.top
    anchors.topMargin: 20

    Rectangle {
      anchors.fill: parent
      color: Qt.rgba(242 / 255, 243 / 255, 245 / 255, 1)
    }

    Components.TopBar {
      id: topBar
    }

    StackLayout {
      anchors.top: topBar.bottom
      anchors.topMargin: 20
      anchors.left: parent.left
      anchors.leftMargin: 16
      currentIndex: topBar.activeTab

      Pages.DevicesPage {}

      Pages.ScenariosPage {}

      Pages.SettingsPage {}
    }
  }
}
