import QtQuick
import YandexHomeDesktop.Ui as UI

UI.PageSurface {
  UI.MessageActionsPane {
    anchors.fill: parent
    message: qsTr("Не удалось получить доступ к хранилищу!")
    messagePixelSize: 16
    detail: qsTr("Это необходимо для работы приложения")
    primaryButtonText: qsTr("Попробовать ещё раз")
    onPrimaryClicked: router.navigateTo("loading")
  }
}
