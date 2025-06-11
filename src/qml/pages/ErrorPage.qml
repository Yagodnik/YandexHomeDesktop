import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  Rectangle {
    anchors.fill: parent
    color: Qt.rgba(242 / 255, 243 / 255, 245 / 255, 1)
  }

  Column {
    id: col
    anchors.centerIn: parent
    spacing: 15

    UI.AnimatedText {
      text: qsTr("Что-то пошло не так!")
      color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1)
      pointSize: 24

      anchors.horizontalCenter: col.horizontalCenter
    }

    UI.MyButton {
      text: "Попробовать ещё раз"

      anchors.horizontalCenter: col.horizontalCenter

      onClicked: {
        router.navigateTo("auth")
      }
    }
  }
}