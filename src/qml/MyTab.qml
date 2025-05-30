import QtQuick

Item {
  id: root

  property var leftCorner: false
  property var rightCorner: false

  property var text: "My Tab"
  property var active: false

  property var tabHeight: 40
  property var inactiveColor: Qt.rgba(145/255, 156/255, 181/255, 1)
  property var activeColor: Qt.rgba(104/255, 58/255, 207/255, 1)
  property var currentColor: active ? activeColor : inactiveColor

  signal clicked

  width: 90
  height: tabHeight

  // implicitWidth: layout.implicitWidth + (2 * layout.anchors.margins)

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor

    onClicked: {
      root.clicked();
    }
  }

  Item {
    id: layout
    anchors.fill: parent
    anchors.margins: 8

    Text {
      text: root.text
      font.bold: true
      anchors.centerIn: parent

      color: root.currentColor

      verticalAlignment: Text.AlignVCenter
    }
  }

  Rectangle {
    id: under
    height: 2

    opacity: active ? 1.0 : 0.0

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: root.leftCorner ? 0 : 5
    anchors.rightMargin: root.rightCorner ? 0 : 5
    anchors.bottom: parent.bottom

    color: root.currentColor

    radius: 2
  }
}