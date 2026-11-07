import QtQuick
import YandexHomeDesktop.Ui as UI

UI.PageSurface {
  UI.LoadingPane {
    anchors.fill: parent
    active: true
    indicatorSize: 48
    strokeWidth: 3
    message: qsTr("Загрузка...")
  }

  Component.onCompleted: authorizationService.AttemptLocalAuthorization()
}
