import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  property string message
  property string detail
  property int messagePixelSize: 24
  property int detailPixelSize: 12
  property string primaryButtonText
  property string secondaryButtonText
  property bool equalButtonWidths: false
  signal primaryClicked()
  signal secondaryClicked()

  Column {
    id: content
    anchors.centerIn: parent
    spacing: 15

    Column {
      id: messages
      anchors.horizontalCenter: parent.horizontalCenter
      spacing: 5

      UI.AnimatedText {
        text: root.message
        color: themes.inactive
        pixelSize: root.messagePixelSize
        wrapWidth: Math.max(0, root.width - 32)
        anchors.horizontalCenter: parent.horizontalCenter
      }

      UI.AnimatedText {
        text: root.detail
        visible: root.detail.length !== 0
        color: themes.inactive
        pixelSize: root.detailPixelSize
        wrapWidth: Math.max(0, root.width - 32)
        anchors.horizontalCenter: parent.horizontalCenter
      }
    }

    Column {
      anchors.horizontalCenter: parent.horizontalCenter
      spacing: 8
      property real buttonWidth: Math.max(messages.implicitWidth,
        primaryButton.implicitWidth, secondaryButton.implicitWidth)

      UI.MyButton {
        id: primaryButton
        width: root.equalButtonWidths ? parent.buttonWidth : implicitWidth
        visible: root.primaryButtonText.length !== 0
        text: root.primaryButtonText
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: root.primaryClicked()
      }

      UI.MyButton {
        id: secondaryButton
        width: root.equalButtonWidths ? parent.buttonWidth : implicitWidth
        visible: root.secondaryButtonText.length !== 0
        text: root.secondaryButtonText
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: root.secondaryClicked()
      }
    }
  }
}
