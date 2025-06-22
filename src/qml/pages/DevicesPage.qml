import QtQuick
import QtQuick.Layouts
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
    devicesModel.RequestData();
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

    Item {
      UI.MyProgressIndicator {
        id: loadingProgress

        anchors.centerIn: parent
        width: 30
        height: 30
        strokeWidth: 2
        opacity: (devicesStack.currentIndex === 0) ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
          NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
          }
        }
      }
    }

    Item {
      opacity: (devicesStack.currentIndex === 1) ? 1 : 0
      visible: opacity > 0

      Behavior on opacity {
        NumberAnimation {
          duration: 200
          easing.type: Easing.InOutQuad
        }
      }

      Column {
        id: errorMessage
        anchors.centerIn: parent
        spacing: 15

        UI.DefaultText {
          text: qsTr("Что-то пошло не так!")
          color: themes.GetInactive()

          anchors.horizontalCenter: errorMessage.horizontalCenter
        }
      }
    }

    Item {
      // TODO: Add this
      // UI.DefaultText {
      //   anchors.centerIn: parent
      //   text: "Пока что у вас нет устройств"
      //
      //   visible: deviceModel.count === 0
      // }

      ListView {
        id: roomsList
        anchors.fill: parent

        clip: true

        spacing: 8

        model: RoomsFilterModel {
          sourceModel: roomsModel
          householdId: "393c92a2-000d-4ae4-b2dc-f3cd0dc00188"
        }

        delegate: Components.RoomDevicesList {
          width: roomsList.width
        }
      }
    }
  }
}
