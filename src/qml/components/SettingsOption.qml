import QtQuick 2.15
import YandexHomeDesktop.Ui as UI

Item {
  id: root

  width: parent.width
  height: details2.height + 20

  property string optionName
  property string optionDescription

  signal toggled(checked: bool)

  Column {
    id: details2
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.verticalCenter: parent.verticalCenter

    UI.DefaultText {
      text: root.optionName
    }

    UI.SubheadingText {
      text: root.optionDescription
    }
  }

  UI.MySwitch {
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.verticalCenter: parent.verticalCenter
    checked: settings.trayModeEnabled

    onToggled: (checked) => root.toggled(checked)
  }
}