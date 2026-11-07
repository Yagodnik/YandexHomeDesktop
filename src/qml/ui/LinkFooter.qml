import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 48
  property string text
  property url iconSource
  signal clicked()

  Row {
    anchors.centerIn: parent
    spacing: 8

    Image {
      source: root.iconSource
      antialiasing: true
      layer.enabled: true
      layer.smooth: true
      layer.samples: 8
      fillMode: Image.PreserveAspectFit
      scale: 0.9
      anchors.verticalCenter: parent.verticalCenter
    }

    UI.LinkLabel {
      text: root.text
      color: themes.inactive
      anchors.verticalCenter: parent.verticalCenter
      onClicked: root.clicked()
    }
  }
}
