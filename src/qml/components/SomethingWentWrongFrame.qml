import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  property bool showCondition

  opacity: showCondition ? 1 : 0
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
      text: qsTr("Что-то пошло не так!")
      color: themes.inactive

      anchors.horizontalCenter: errorMessage.horizontalCenter
    }
  }
}