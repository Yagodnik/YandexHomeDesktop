import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root

  property bool active: false
  property string message

  opacity: active ? 1 : 0
  visible: opacity > 0

  Behavior on opacity {
    NumberAnimation {
      duration: 200
      easing.type: Easing.InOutQuad
    }
  }

  Column {
    id: errorMessage
    anchors.centerIn: parent
    spacing: 15

    UI.DefaultText {
      text: root.message
      color: themes.inactive
      anchors.horizontalCenter: errorMessage.horizontalCenter
    }
  }
}
