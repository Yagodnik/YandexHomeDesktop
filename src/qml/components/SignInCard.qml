import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitWidth: content.implicitWidth + 40
  implicitHeight: content.implicitHeight + 40
  property string title
  property string message
  property string secondaryButtonText
  property string buttonText
  signal signInRequested()
  signal cancelRequested()

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
      wrapWidth: 300
      color: themes.inactive
      pixelSize: 16
      anchors.horizontalCenter: parent.horizontalCenter
    }

    UI.MyButton {
      objectName: "signInAction"
      visible: root.buttonText.length > 0
      text: root.buttonText
      anchors.horizontalCenter: parent.horizontalCenter
      onClicked: root.signInRequested()
    }

    UI.MyButton {
      objectName: "cancelSignInAction"
      visible: root.secondaryButtonText.length > 0
      text: root.secondaryButtonText
      anchors.horizontalCenter: parent.horizontalCenter
      onClicked: root.cancelRequested()
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
