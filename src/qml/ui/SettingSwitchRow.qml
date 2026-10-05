import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: details.implicitHeight + 20
  property string title
  property string description
  property bool checked: false
  signal toggled(bool checked)

  Column {
    id: details
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.verticalCenter: parent.verticalCenter

    UI.DefaultText { text: root.title }
    UI.SubheadingText { text: root.description }
  }

  UI.MySwitch {
    objectName: "settingSwitch"
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.verticalCenter: parent.verticalCenter
    checked: root.checked
    onToggled: function(checked) { root.toggled(checked); }
  }
}
