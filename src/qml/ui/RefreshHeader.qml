import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 32

  property string title
  property alias rotationAngle: reloadButton.rotationAngle

  signal refreshClicked()

  UI.HeadingText {
    text: root.title
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
  }

  UI.ImageButton {
    id: reloadButton
    source: "qrc:/images/reload.svg"

    property real rotationAngle: 0

    transform: Rotation {
      origin.x: reloadButton.width / 2
      origin.y: reloadButton.height / 2
      angle: reloadButton.rotationAngle
    }

    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: 8

    onClicked: root.refreshClicked()

    Behavior on rotationAngle {
      NumberAnimation {
        duration: 500
        easing.type: Easing.InOutCubic
      }
    }
  }
}
