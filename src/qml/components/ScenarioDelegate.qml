import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  id: delegateItem
  height: 48

  opacity: 0

  Behavior on opacity {
    NumberAnimation {
      duration: 300
      easing.type: Easing.InOutQuad
    }
  }

  Component.onCompleted: {
    opacity = 1
  }

  Rectangle {
    id: darkOverlay
    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.1)
    visible: !is_active
    radius: 16
    z: 100

    MouseArea {
      anchors.fill: parent
      enabled: true
      hoverEnabled: true
      preventStealing: true
    }

    Behavior on opacity {
      NumberAnimation { duration: 150 }
    }

    opacity: visible ? 1 : 0
  }

  DropShadow {
    anchors.fill: background
    source: background
    horizontalOffset: 0
    verticalOffset: 2
    radius: 6
    samples: 16
    color: Qt.rgba(0, 32 / 255, 128 / 255, 0.04)
  }

  Rectangle {
    id: background
    anchors.fill: parent
    color: themes.headerBackground
    radius: 16
  }

  UI.DefaultText {
    text: name
    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 16
  }

  property var show: false

  UI.MyProgressIndicator {
    width: 30
    height: 30
    strokeWidth: 2
    anchors.right: parent.right
    anchors.rightMargin: 9
    anchors.verticalCenter: parent.verticalCenter
    opacity: is_waiting_response ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }
  }

  Image {
    id: startButton
    source: "qrc:/images/play.svg"

    opacity: is_waiting_response ? 0 : 1
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 100
        easing.type: Easing.InOutQuad
      }
    }

    antialiasing: true
    layer.enabled: true
    layer.smooth: true
    layer.samples: 8

    fillMode: Image.PreserveAspectFit
    scale: 0.5

    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: 8

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor

      onClicked: {
        console.log("Executing " + scenario_id);
        scenariosModel.ExecuteScenario(index);
      }
    }
  }
}