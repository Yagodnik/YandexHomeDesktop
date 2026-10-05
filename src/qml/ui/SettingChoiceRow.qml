import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: 50
  property string title
  property var choices
  property int currentIndex: 0
  signal activated(int index)

  UI.DefaultText {
    text: root.title
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.verticalCenter: parent.verticalCenter
  }

  UI.MyComboBox {
    objectName: "settingChoice"
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.verticalCenter: parent.verticalCenter
    currentIndex: root.currentIndex
    model: root.choices
    onActivated: function(index) { root.activated(index); }
  }
}
