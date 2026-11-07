import QtQuick
import QtQuick.Controls

ToolButton {
  id: root
  implicitWidth: 36
  implicitHeight: 36
  display: AbstractButton.IconOnly
  property color foregroundColor: themes.mainText
  icon.width: 20
  icon.height: 20
  icon.color: foregroundColor
  Accessible.name: text
  opacity: enabled ? 1 : 0.4
  HoverHandler { cursorShape: Qt.PointingHandCursor }
  background: Rectangle {
    radius: 6
    color: Qt.rgba(root.foregroundColor.r, root.foregroundColor.g, root.foregroundColor.b,
                   root.down ? 0.22 : root.hovered ? 0.12 : 0)
    border.width: root.visualFocus ? 2 : 0
    border.color: root.foregroundColor
  }
  ToolTip.visible: hovered
  ToolTip.delay: 500
  ToolTip.text: text
}
