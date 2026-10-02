import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 48
  z: 200

  property string title

  signal backClicked()

  Rectangle {
    id: headerBackground
    anchors.fill: parent
    color: themes.headerBackground
  }

  DropShadow {
    anchors.fill: headerBackground
    source: headerBackground
    radius: 12
    samples: 16
    horizontalOffset: 0
    verticalOffset: 2
    color: themes.shadowColor
  }

  Image {
    id: backButton
    source: "qrc:/images/back.svg"

    antialiasing: true
    layer.enabled: true
    layer.smooth: true
    layer.samples: 8
    fillMode: Image.PreserveAspectFit

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 12

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      onClicked: root.backClicked()
    }
  }

  UI.DefaultText {
    text: root.title
    anchors.verticalCenter: parent.verticalCenter
    anchors.horizontalCenter: parent.horizontalCenter
    color: themes.controlText
  }
}
