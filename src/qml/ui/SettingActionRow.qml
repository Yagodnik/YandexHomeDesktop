import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  property string title
  property string description
  property string buttonText
  signal triggered()
  implicitHeight: Math.max(details.implicitHeight, action.implicitHeight) + 20

  Column {
    id: details
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.right: action.left
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter
    spacing: 4

    UI.DefaultText {
      width: parent.width
      text: root.title
      wrapMode: Text.WordWrap
    }
    UI.SubheadingText {
      width: parent.width
      text: root.description
      wrapMode: Text.WordWrap
    }
  }

  UI.MyButton {
    id: action
    objectName: "settingActionButton"
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.verticalCenter: parent.verticalCenter
    text: root.buttonText
    font.pixelSize: 14
    horizontalMargin: 10
    verticalMargin: 4
    opacity: enabled ? 1 : 0.4
    onClicked: root.triggered()
  }
}
