import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  Rectangle {
    anchors.fill: parent
    color: themes.background
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
        color: themes.inactive
        pointSize: 16

        anchors.horizontalCenter: parent.horizontalCenter
      }

      UI.AnimatedText {
        text: qsTr("Это необходимо для работы приложения")
        color: themes.inactive
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