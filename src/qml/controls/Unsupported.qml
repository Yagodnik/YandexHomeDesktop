import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 48

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
    radius: 16
  }

  UI.DefaultText {
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    text: "Неподдерживаемое умение: " + name
  }
}