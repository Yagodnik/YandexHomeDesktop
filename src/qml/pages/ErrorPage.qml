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

    UI.AnimatedText {
      text: "Что-то пошло не так! " + authorizationService.GetLastErrorCode();
      color: themes.inactive
      pixelSize: 24

      anchors.horizontalCenter: col.horizontalCenter
    }

    Column {
      spacing: 8
      anchors.horizontalCenter: col.horizontalCenter

      UI.MyButton {
        width: col.width
        text: "Попробовать ещё раз"

        anchors.horizontalCenter: col.horizontalCenter

        onClicked: {
          router.navigateTo("loading");
        }
      }

      UI.MyButton {
        width: col.width
        text: "Выйти из аккаунта"

        anchors.horizontalCenter: col.horizontalCenter

        onClicked: {
          authorizationService.Logout();
          router.navigateTo("auth");
        }
      }
    }
  }
}