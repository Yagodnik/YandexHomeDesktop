import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  Components.SignInCard {
    anchors.centerIn: parent
    title: qsTr("Yandex Home Desktop")
    message: authorizationService.authorizationPending
      ? (authorizationService.authorizationCode.length > 0
         ? qsTr("Откройте %1\nи введите код: %2").arg(authorizationService.authorizationUrl).arg(authorizationService.authorizationCode)
         : qsTr("Получение кода авторизации…"))
      : qsTr("Необходимо войти в аккаунт, чтобы\nприложение могло получить доступ\nк вашим устройствам")
    buttonText: authorizationService.authorizationPending
      ? (authorizationService.authorizationCode.length > 0 ? qsTr("Открыть страницу") : "")
      : qsTr("Авторизоваться")
    secondaryButtonText: authorizationService.authorizationPending ? qsTr("Отмена") : ""
    onSignInRequested: {
      if (authorizationService.authorizationPending)
        Qt.openUrlExternally(authorizationService.authorizationUrl)
      else
        authorizationService.AttemptAuthorization()
    }
    onCancelRequested: authorizationService.CancelAuthorization()
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
