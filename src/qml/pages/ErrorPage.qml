import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  Rectangle {
    anchors.fill: parent
    color: themes.GetBackground()
  }

  Column {
    id: col
    anchors.centerIn: parent
    spacing: 15

    UI.AnimatedText {
      text: qsTr("Что-то пошло не так!")
      color: themes.GetInactive()
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