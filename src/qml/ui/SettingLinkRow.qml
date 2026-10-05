import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: 48
  property string text
  signal clicked()

  UI.LinkLabel {
    text: root.text
    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 10
    onClicked: root.clicked()
  }
}
