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
    loading: true

    model: householdsModel

    loadingDelegate: Components.SelectDialogLoadingIndicator {
      loading: householdSelectDialog.loading
    }

    delegate: Components.SelectDialogItem {
      width: parent.width

      selected: model.householdId === householdsModel.currentHousehold
      elementsCount: householdsModel.count

      onClicked: {
        householdsModel.currentHousehold = model.householdId;
        householdSelectDialog.close();
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

    Components.AppHeader {
      id: appHeader

      showHouseholdSelector: true
    }

    Item {
      clip: true
      anchors.top: appHeader.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      StackLayout {
        anchors.fill: parent
        anchors.topMargin: 16
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.bottomMargin: 16

        currentIndex: appHeader.activeTabIndex

        Pages.DevicesPage {}

        Pages.ScenariosPage {}

        Pages.SettingsPage {}
      }
    }
  }
}
