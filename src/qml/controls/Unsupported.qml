import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 48

  UI.CardSurface {
    anchors.fill: parent
  }

  UI.DefaultText {
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    text: qsTr("Неподдерживаемое умение: %1").arg(name)
  }
}
