import QtQuick
import YandexHomeDesktop.Ui as UI

UI.DefaultText {
  id: root
  signal clicked()

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor
    onClicked: root.clicked()
  }
}
