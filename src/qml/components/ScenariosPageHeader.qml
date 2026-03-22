import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: header
  width: parent.width
  height: 32

  signal reloadClicked

  UI.HeadingText {
    id: headerTitle
    text: "Все сценарии"
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
  }

  function playRotateAnimation() {
    reloadButton.rotationAngle += 360;
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
      header.reloadClicked();
      playRotateAnimation();
    }

    Behavior on rotationAngle {
      NumberAnimation {
        duration: 500
        easing.type: Easing.InOutCubic
      }
    }
  }
}