import QtQuick
import QtQuick.Controls

AbstractButton {
  id: root
  height: 36
  required property string title
  required property bool collapsed
  property bool animate: false
  Component.onCompleted: animate = true
  signal toggleRequested()
  Accessible.name: collapsed ? qsTr("Развернуть %1").arg(title) : qsTr("Свернуть %1").arg(title)
  Accessible.role: Accessible.Button
  hoverEnabled: true
  onClicked: toggleRequested()
  HoverHandler { cursorShape: Qt.PointingHandCursor }

  contentItem: Item {
    Text {
      id: roomTitle
      objectName: "sectionTitle"
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      width: Math.min(implicitWidth, Math.max(0, parent.width - arrow.width - 8))
      text: root.title
      color: themes.inactive
      font.pointSize: 14
      font.bold: true
      elide: Text.ElideRight
    }

    ToolButton {
      id: arrow
      objectName: "sectionArrow"
      width: 24
      height: 24
      anchors.left: roomTitle.right
      anchors.leftMargin: 8
      anchors.verticalCenter: parent.verticalCenter
      padding: 0
      background: null
      icon.source: "qrc:/images/arrow.svg"
      icon.width: 20
      icon.height: 20
      icon.color: root.hovered ? themes.accent : themes.inactive
      display: AbstractButton.IconOnly
      focusPolicy: Qt.NoFocus
      Accessible.ignored: true
      rotation: root.collapsed ? 90 : 180
      onClicked: root.toggleRequested()

      Behavior on rotation {
        enabled: root.animate
        NumberAnimation { duration: 220; easing.type: Easing.InOutCubic }
      }
    }
  }

  background: Rectangle {
    color: "transparent"
    radius: 8
    border.width: root.visualFocus ? 2 : 0
    border.color: themes.accent
  }
}
