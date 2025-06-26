import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  Rectangle {
    anchors.fill: parent
    color: themes.background
  }

  UI.MyProgressIndicator {
    id: progress
    width: 48
    height: 48
    anchors.centerIn: parent
    anchors.horizontalCenter: parent.horizontalCenter
  }

  UI.DefaultText {
    anchors.top: progress.bottom
    anchors.topMargin: 15
    anchors.horizontalCenter: parent.horizontalCenter
    color: themes.accent

    text: "Загрузка..."
  }

  Component.onCompleted: {
    authorizationService.AttemptLocalAuthorization();
  }

  // UI.MyButton {
  //   text: "Nav"
  //
  //   onClicked: {
  //     router.navigateTo("auth");
  //   }
  // }
}