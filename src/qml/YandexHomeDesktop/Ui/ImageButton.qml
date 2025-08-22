import QtQuick

Image {
  id: imageButton

  signal clicked

  antialiasing: true
  layer.enabled: true
  layer.smooth: true
  layer.samples: 8

  fillMode: Image.PreserveAspectFit

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor

    onClicked: imageButton.clicked()
  }
}