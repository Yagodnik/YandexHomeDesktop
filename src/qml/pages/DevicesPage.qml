import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models 1.0

UI.PageSurface {
  id: rooms

  UI.RefreshHeader {
    id: heading
    width: parent.width
    title: qsTr("Комнаты")

    onRefreshClicked: {
      if (rooms.isLoading) {
        return;
      }

      rooms.isLoading = true;
      devicesStack.currentIndex = 0;
      devicesModel.RequestData();

      heading.rotationAngle += 360;
    }
  }

  Component.onCompleted: {
    isLoading = true;
    devicesModel.RequestData();
  }

  property bool isLoading: false

  Connections {
    target: devicesModel

    function onDataLoadingFailed() {
      rooms.isLoading = false;
      console.log("Devices Model: Data Load - FAIL");
      devicesStack.currentIndex = 1;
    }

    function onDataLoaded() {
      rooms.isLoading = false;
      console.log("Devices Model: Data Load - OK");
      devicesStack.currentIndex = 2;
    }
  }

  UI.PageStates {
    id: devicesStack
    width: parent.width

    anchors.top: heading.bottom
    anchors.topMargin: 2
    anchors.bottom: parent.bottom

    UI.LoadingPane {
      active: devicesStack.currentIndex === 0
    }

    UI.LoadErrorPane {
      active: devicesStack.currentIndex === 1
      message: qsTr("Что-то пошло не так!")
    }

    Components.RoomsPane {
      sourceModel: RoomsFilterModel {
        sourceModel: roomsModel
        householdId: householdsModel.currentHousehold
      }
    }
  }
}
