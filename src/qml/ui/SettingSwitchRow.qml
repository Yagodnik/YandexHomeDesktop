import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: Math.max(details.implicitHeight, control.height) + 20
  property string title
  property string description
  property bool checked: false
  signal toggled(bool checked)

  Column {
    id: details
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.right: control.left
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter
    spacing: 4

    UI.DefaultText {
      width: parent.width
      text: root.title
      wrapMode: Text.Wrap
    }
    UI.SubheadingText {
      width: parent.width
      text: root.description
      wrapMode: Text.Wrap
    }
  }

  UI.MySwitch {
    id: control
    objectName: "settingSwitch"
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.verticalCenter: parent.verticalCenter
    checked: root.checked
    optimistic: false
    onToggled: function(checked) { root.toggled(checked); }
  }
}
