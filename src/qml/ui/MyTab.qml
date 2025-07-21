import QtQuick

Item {
  id: root

  property var leftCorner: false
  property var rightCorner: false

  property var text: "My Tab"
  property var active: false

  property var tabHeight: 48
  property var inactiveColor: themes.inactive
  property var activeColor: themes.accent
  property var currentColor: active ? activeColor : inactiveColor

  signal clicked

  width: tabText.contentWidth
  height: tabHeight

  // implicitWidth: layout.implicitWidth + (2 * layout.anchors.margins)

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor

    onClicked: {
      root.clicked();
    }
  }

  Text {
    id: tabText
    text: root.text
    font.bold: true
    font.pixelSize: 14

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    color: root.currentColor
  }

  Rectangle {
    id: under
    height: 2
    width: tabText.width

    anchors.horizontalCenter: tabText.horizontalCenter
    anchors.bottom: parent.bottom

    opacity: active ? 1.0 : 0.0
    color: root.currentColor
    radius: 2
  }
}
