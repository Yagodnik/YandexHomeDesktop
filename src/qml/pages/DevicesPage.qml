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
      }

      ListElement {
        name: "Лампа"
      }

      ListElement {
        name: "Сарайчик"
      }
    }

    delegate: Components.RoomDevicesList {}
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
