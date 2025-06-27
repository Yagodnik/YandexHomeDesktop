import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
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
      onClicked: householdSelectDialog.close()
      enabled: true
    }
  }

  Connections {
    target: householdsModel

    function onDataLoadingFailed() {
      householdSelectDialog.loading = false;
      console.log("Households Model: Data Load - FAIL");
    }

    function onDataLoaded() {
      householdSelectDialog.loading = false;
      console.log("Households Model: Data Load - OK");
    }
  }

  UI.SelectDialog {
    id: householdSelectDialog

    title: "Выберите Дом"

    model: householdsModel

    loading: true

    loadingDelegate: Item {
      UI.MyProgressIndicator {
        id: loadingProgress

        anchors.centerIn: parent

        width: 30
        height: 30
        strokeWidth: 2
        opacity: householdSelectDialog.loading ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
          NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
          }
        }
      }
    }

    delegate: Column {
      width: parent.width
      spacing: 6

      Item {
        width: parent.width
        height: 32

        Image {
          id: householdIcon
          width: 24
          height: 24
          anchors.verticalCenter: parent.verticalCenter
          anchors.left: parent.left
          source: "qrc:/images/household.svg"
        }

        ColorOverlay {
          anchors.fill: householdIcon
          source: householdIcon
          color: model.householdId === householdsModel.currentHousehold ? themes.accent : themes.inactive
        }

        UI.DefaultText {
          id: houseName
          text: model.name
          anchors.left: householdIcon.right
          anchors.leftMargin: 12
          anchors.verticalCenter: parent.verticalCenter
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor

          onClicked: {
            householdsModel.currentHousehold = model.householdId;
            householdSelectDialog.close();
          }
        }
      }

      Rectangle {
        visible: model.index < householdsModel.count - 1
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 36
        color: themes.inactive
      }
    }
  }

  Item {
    id: appRoot

    anchors.fill: parent

    Rectangle {
      anchors.fill: parent
      color: themes.background
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