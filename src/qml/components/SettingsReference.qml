import QtQuick 2.15
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  width: parent.width
  height: 48

  property string optionName
  property string optionUrl

  UI.DefaultText {
    id: githubLink

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 10

    text: root.optionName

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor

      onClicked: {
        Qt.openUrlExternally(root.optionUrl)
      }
    }
  }
}