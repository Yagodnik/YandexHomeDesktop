import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: Math.max(label.implicitHeight, control.height) + 20
  property string title
  property var choices
  property int currentIndex: 0
  signal activated(int index)

  UI.DefaultText {
    id: label
    text: root.title
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.right: control.left
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter
    elide: Text.ElideRight
  }

  UI.MyComboBox {
    id: control
    objectName: "settingChoice"
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.verticalCenter: parent.verticalCenter
    currentIndex: root.currentIndex
    model: root.choices
    onActivated: function(index) { root.activated(index); }
  }
}
