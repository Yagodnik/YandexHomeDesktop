import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels as ViewModels

UI.PageSurface {
  id: root
  required property ViewModels.HomeViewModel viewModel

  Component.onCompleted: viewModel.EnsureLoaded()

  UI.RefreshHeader {
    id: heading
    objectName: "homeRefreshButton"
    width: parent.width
    title: qsTr("Комнаты")
    enabled: !root.viewModel.loading

    onRefreshClicked: {
      root.viewModel.Refresh();
      heading.rotationAngle += 360;
    }
  }

  UI.PageStates {
    id: devicesStack
    objectName: "homeStates"
    width: parent.width
    anchors.top: heading.bottom
    anchors.topMargin: 2
    anchors.bottom: parent.bottom
    currentIndex: root.viewModel.state === ViewModels.HomeViewModel.Ready ? 2
                : root.viewModel.state === ViewModels.HomeViewModel.Error ? 1 : 0

    UI.LoadingPane { active: devicesStack.currentIndex === 0 }
    UI.LoadErrorPane {
      active: devicesStack.currentIndex === 1
      message: qsTr("Что-то пошло не так!")
    }
    Components.RoomsPane {
      sourceModel: root.viewModel.rooms
      devicesModel: root.viewModel.devices
    }
  }
}
