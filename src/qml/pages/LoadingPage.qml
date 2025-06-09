import QtQuick
import "../ui/" as UI

Item {
  UI.MyProgressIndicator {
    id: progress
    width: 48
    height: 48
    anchors.centerIn: parent
    anchors.horizontalCenter: parent.horizontalCenter
  }

  UI.DefaultText {
    anchors.top: progress.bottom
    anchors.topMargin: 15
    anchors.horizontalCenter: parent.horizontalCenter
    color: "#6839CF"

    text: "Загрузка..."
  }
}