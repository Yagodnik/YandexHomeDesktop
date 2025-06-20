import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  height: 48

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
    radius: 16
  }

  UI.DefaultText {
    anchors.left: parent.left
    anchors.leftMargin: 8
    anchors.verticalCenter: parent.verticalCenter

    text: name
  }

  UI.MySwitch {
    anchors.right: parent.right
    anchors.rightMargin: 8
    anchors.verticalCenter: parent.verticalCenter
  }
}