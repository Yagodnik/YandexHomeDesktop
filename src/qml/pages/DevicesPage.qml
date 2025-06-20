import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

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

    Image {
      id: reloadButton
      source: "qrc:/images/reload.svg"

      property real rotationAngle: 0

      antialiasing: true
      layer.enabled: true
      layer.smooth: true
      layer.samples: 8

      fillMode: Image.PreserveAspectFit
      transform: Rotation {
        id: rot
        origin.x: reloadButton.width / 2
        origin.y: reloadButton.height / 2
        angle: reloadButton.rotationAngle
      }

      anchors.verticalCenter: parent.verticalCenter
      anchors.right: parent.right
      anchors.rightMargin: 8

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor

        onClicked: {
          console.log("Reloading devices")
          reloadButton.rotationAngle += 360;
        }
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
    devicesModel.RequestData();
  }

  ListView {
    width: parent.width

    anchors.top: heading.bottom
    anchors.topMargin: 2
    anchors.bottom: parent.bottom

    clip: true
    // interactive: false

    spacing: 8

    model: ListModel {
      ListElement {
        name: "Завод"
        roomId: "925fadea-bfb9-4ddf-868a-b5ba52e74ac5"
        householdId: "393c92a2-000d-4ae4-b2dc-f3cd0dc00188"
      }

      ListElement {
        name: "Лампа"
        roomId: "75d43bd0-41be-4846-9b9a-a7eba83d2353"
        householdId: "393c92a2-000d-4ae4-b2dc-f3cd0dc00188"
      }
    }
    delegate: Components.RoomDevicesList {}

    // model: devicesModel
    // delegate: Components.DeviceDelegate {}
  }

  // Column {
  //   UI.MyButton {
  //     text: "Do request"
  //
  //     onClicked: {
  //       yandex_api.RequestInfo();
  //     }
  //   }
  //
  //   UI.MyButton {
  //     text: "Show"
  //
  //     onClicked: {
  //       platformService.ShowAsApp();
  //     }
  //   }
  //
  //   UI.MyButton {
  //     text: "Hide"
  //
  //     onClicked: {
  //       platformService.ShowOnlyInTray();
  //     }
  //   }
  // }
}
