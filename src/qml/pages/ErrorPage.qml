import QtQuick
import YandexHomeDesktop.Ui as UI

UI.PageSurface {
  UI.MessageActionsPane {
    anchors.fill: parent
    message: qsTr("Что-то пошло не так! %1").arg(authorizationService.GetLastErrorCode())
    primaryButtonText: qsTr("Попробовать ещё раз")
    secondaryButtonText: qsTr("Выйти из аккаунта")
    equalButtonWidths: true
    onPrimaryClicked: router.navigateTo("loading")
    onSecondaryClicked: authorizationService.Logout()
  }
}
