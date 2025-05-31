import QtQuick

Item {
  id: root

  property var title
  property var model
  property Component delegate
  property var selectedIndex: -1

  visible: true
  anchors.left: parent.left
  anchors.right: parent.right
  anchors.bottom: parent.bottom
  z: 2
  height: contentColumn.implicitHeight + 50

  Rectangle {
    anchors.fill: parent
    color: "white"
    radius: 16
  }

  Text {
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
    anchors.topMargin: 4
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