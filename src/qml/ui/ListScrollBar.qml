import QtQuick
import QtQuick.Controls

ScrollBar {
  required property Flickable view
  width: 10
  height: view.height
  anchors.left: view.right
  anchors.leftMargin: 4
  policy: view.contentHeight > view.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

  contentItem: Rectangle {
    implicitWidth: 10
    radius: 4
    color: themes.headerBackground
  }

  background: Item {}
}
