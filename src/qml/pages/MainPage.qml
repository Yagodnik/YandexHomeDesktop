import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Pages as Pages
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  Connections {
    target: householdsModel

    function onDataLoadingFailed() {
      householdPicker.loading = false;
      console.log("Households Model: Data Load - FAIL");
    }

    function onDataLoaded() {
      householdPicker.loading = false;
      console.log("Households Model: Data Load - OK");
    }
  }

  Components.TopBar {
    id: topBar
    householdName: householdsModel.currentHouseholdName
    onHouseholdSelectRequested: householdPicker.open()
  }

  UI.InsetPane {
    anchors.top: topBar.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    UI.PageStates {
      anchors.fill: parent
      currentIndex: topBar.activeTab

      Pages.DevicesPage {}
      Pages.ScenariosPage {}
      Pages.SettingsPage {}
    }
  }

  Components.HouseholdPicker {
    id: householdPicker
    objectName: "householdPicker"
    anchors.fill: parent
    title: qsTr("Выберите Дом")
    sourceModel: householdsModel
    currentHousehold: householdsModel.currentHousehold

    onHouseholdSelected: function(householdId) {
      householdsModel.currentHousehold = householdId;
      householdPicker.close();
    }
  }
}
