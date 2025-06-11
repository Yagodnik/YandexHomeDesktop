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

    Column {
      anchors.horizontalCenter: col.horizontalCenter
      spacing: 5

      UI.AnimatedText {
        text: qsTr("Не удалось получить доступ к хранилищу!")
        color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1)
        pointSize: 16

        anchors.horizontalCenter: parent.horizontalCenter
      }

      UI.AnimatedText {
        text: qsTr("Это необходимо для работы приложения")
        color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1)
        pointSize: 12

        anchors.horizontalCenter: parent.horizontalCenter
      }
    }

    UI.MyButton {
      text: "Попробовать ещё раз"

      anchors.horizontalCenter: col.horizontalCenter

      onClicked: {
        router.navigateTo("loading");
      }
    }
  }
}