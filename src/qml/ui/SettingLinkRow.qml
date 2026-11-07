import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: label.implicitHeight + 20
  property string text
  signal clicked()

  UI.LinkLabel {
    id: label
    text: root.text
    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 10
    onClicked: root.clicked()
  }
}
