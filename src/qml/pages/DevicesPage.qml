import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models 1.0

Item {
  id: rooms

  Item {
    id: heading
    width: parent.width
    height: 32

    UI.HeadingText {
      id: roomsText
      text: "Комнаты"
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
    }

    UI.ImageButton {
      id: reloadButton
      source: "qrc:/images/reload.svg"

      property real rotationAngle: 0

      transform: Rotation {
        id: rot
        origin.x: reloadButton.width / 2
        origin.y: reloadButton.height / 2
        angle: reloadButton.rotationAngle
      }

      anchors.verticalCenter: parent.verticalCenter
      anchors.right: parent.right
      anchors.rightMargin: 8

      onClicked: {
        if (rooms.isLoading) {
          return;
        }

        rooms.isLoading = true;
        devicesStack.currentIndex = 0;
        devicesModel.RequestData();

        reloadButton.rotationAngle += 360;
      }

      Behavior on rotationAngle {
        NumberAnimation {
          duration: 500
          easing.type: Easing.InOutCubic
        }
      }
    }
  }

  Component.onCompleted: {
    isLoading = true;

    // Now we need to track status of authorization
    // devicesModel.RequestData();

    if (authorizationService.IsAuthorized()) {
      console.log("User authorized -> requesting data");
      devicesModel.RequestData();
    } else {
      rooms.isLoading = false;
      console.log("User unauthorized -> showing instructions");
      devicesStack.currentIndex = 3;
    }
  }

  property var isLoading: false

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

  StackLayout {
    id: devicesStack
    width: parent.width

    anchors.top: heading.bottom
    anchors.topMargin: 2
    anchors.bottom: parent.bottom

    Components.LoadingFrame {
      showCondition: (devicesStack.currentIndex === 0)
    }

    Components.SomethingWentWrongFrame {
      showCondition: (devicesStack.currentIndex === 1)
    }

    Item {
      Item {
        id: wholeDevicesList
        anchors.fill: parent

        UI.MyInputField {
          id: searchField
          width: parent.width
          height: 32

          placeholderText: "Введите название устройства"

          onTextChanged: () => {
            roomsList.searchQuery = searchField.text
          }
        }

        Flickable {
          anchors.top: searchField.bottom
          anchors.topMargin: 2
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          clip: true
          contentHeight: favoriteAndAllDevices.height

          Column {
            id: favoriteAndAllDevices
            width: parent.width
            spacing: 4

            Components.RoomDevicesList {
              id: favoriteList
              width: roomsList.width
              name: "Избранные"
              collapsed: settingsStorage.Contains("collapsed-rooms", "yhd-favorites")
              roomId: "yhd-favorites"

              sourceModel: FavoriteDevicesModel {
                sourceModel: devicesModel
                settingsStorage2: settingsStorage
              }
            }

            ListView {
              id: roomsList
              spacing: 8

              width: parent.width
              height: contentHeight
              interactive: false

              ScrollBar.vertical: scrollBar

              model: RoomsFilterModel {
                sourceModel: roomsModel
                householdId: householdsModel.currentHousehold
              }

              delegate: Components.RoomDevicesList {
                width: roomsList.width
                name: model.name
                collapsed: model.collapsed
                roomId: model.roomId

                sourceModel: DevicesFilterModel {
                  sourceModel: devicesModel
                  householdId: model.householdId
                  roomId: model.roomId
                }
              }
            }
          }
        }
      }

      ScrollBar {
        id: scrollBar

        width: 10
        height: roomsList.height
        anchors.left: parent.right
        anchors.leftMargin: 4
        policy: roomsList.contentHeight > roomsList.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

        contentItem: Rectangle {
          implicitWidth: 10
          radius: 4
          color: themes.headerBackground
        }

        background: Item {}
      }
    }

    Components.RequiresAuthorizationFrame {
      showCondition: (devicesStack.currentIndex === 3)
    }
  }
}
