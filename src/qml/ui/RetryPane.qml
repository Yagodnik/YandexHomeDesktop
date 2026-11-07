import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  property string message
  property string buttonText
  property url iconSource
  signal retryRequested()

  Column {
    anchors.centerIn: parent
    spacing: 4

    Image {
      width: 32
      height: 32
      source: root.iconSource
      visible: root.iconSource.toString().length !== 0
      anchors.horizontalCenter: parent.horizontalCenter
    }

    UI.DefaultText {
      text: root.message
      anchors.horizontalCenter: parent.horizontalCenter
    }

    UI.MyButton {
      objectName: "retryAction"
      text: root.buttonText
      anchors.horizontalCenter: parent.horizontalCenter
      onClicked: root.retryRequested()
    }
  }
}
