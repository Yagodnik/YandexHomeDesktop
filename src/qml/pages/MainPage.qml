import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Pages as Pages
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  popupMenuInHeader: true
  modalDialogOpen: settingsPage.modalDialogOpen
  Components.TopBar {
    id: topBar
    householdName: homeViewModel.currentHouseholdName
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

      Pages.DevicesPage { viewModel: homeViewModel }
      Pages.ScenariosPage {
        viewModel: scenariosViewModel
        keybindSettings: keybindSettingsModel
        showKeybinds: settings.showKeybinds
      }
      Pages.SettingsPage { id: settingsPage; restModel: restViewModel; transferModel: settingsTransferModel; shortcutModel: shortcutViewModel; mcpModel: typeof mcpViewModel === "undefined" ? null : mcpViewModel }
    }
  }

  Components.HouseholdPicker {
    id: householdPicker
    objectName: "householdPicker"
    anchors.fill: parent
    title: qsTr("Выберите Дом")
    sourceModel: homeViewModel.households
    loading: homeViewModel.loading
    currentHousehold: homeViewModel.currentHousehold

    onHouseholdSelected: function(householdId) {
      homeViewModel.SelectHousehold(householdId);
      householdPicker.close();
    }
  }
}
