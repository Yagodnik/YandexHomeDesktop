import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root

  property var title
  property var model
  property Component delegate
  property var selectedIndex: -1

  property var myVisible: false

  visible: true
  anchors.left: parent.left
  anchors.right: parent.right
  // anchors.bottom: parent.bottom
  z: 600
  height: contentColumn.implicitHeight + 60

  y: parent.height

  Behavior on y {
    NumberAnimation {
      duration: 300
      easing.type: Easing.InOutQuad
    }
  }

  onMyVisibleChanged: {
    if (myVisible) {
      root.y = parent.height - root.height // slide in
    } else {
      root.y = parent.height              // slide out
    }
  }

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
    radius: 16
  }

  UI.HeadingText {
    id: selectTitle
    text: title

    font.pointSize: 24
    font.bold: true

    anchors.left: parent.left
    anchors.leftMargin: 16

    anchors.top: parent.top
    anchors.topMargin: 16
  }

  Column {
    id: contentColumn
    anchors.top: selectTitle.bottom
    anchors.topMargin: 12
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16
    spacing: 8

    Repeater {
      model: root.model
      delegate: root.delegate
    }
  }
}