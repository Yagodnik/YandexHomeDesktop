import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  Components.SignInCard {
    anchors.centerIn: parent
    title: qsTr("Yandex Home Desktop")
    message: qsTr("Необходимо войти в аккаунт, чтобы\nприложение могло получить доступ\nк вашим устройствам")
    buttonText: qsTr("Авторизоваться")
    onSignInRequested: authorizationService.AttemptAuthorization()
  }

  UI.LinkFooter {
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    text: qsTr("GitHub")
    iconSource: "qrc:/images/login_github.svg"
    onClicked: Qt.openUrlExternally("https://github.com/Yagodnik/YandexHomeDesktop")
  }
}
