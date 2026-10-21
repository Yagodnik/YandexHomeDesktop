import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitWidth: content.implicitWidth + 40
  implicitHeight: content.implicitHeight + 40
  property string title
  property string message
  property string buttonText
  signal signInRequested()

  UI.CardSurface {
    id: background
    anchors.fill: parent
  }

  Column {
    id: content
    anchors.centerIn: parent
    spacing: 10

    UI.AnimatedText {
      text: root.title
      color: themes.accent
      pixelSize: 24
      anchors.horizontalCenter: parent.horizontalCenter
    }

    UI.AnimatedText {
      text: root.message
      color: themes.inactive
      pixelSize: 16
      anchors.horizontalCenter: parent.horizontalCenter
    }

    UI.MyButton {
      objectName: "signInAction"
      text: root.buttonText
      anchors.horizontalCenter: parent.horizontalCenter
      onClicked: root.signInRequested()
    }
  }

  UI.Shadow {
    anchors.fill: background
    source: background
    horizontalOffset: 0
    verticalOffset: 2
    radius: 6
    samples: 16
    color: themes.shadowColor
  }
}
