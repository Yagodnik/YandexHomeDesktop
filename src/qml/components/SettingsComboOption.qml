import QtQuick 2.15
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  width: parent.width
  height: 50

  property string optionName
  property int currentIndex
  property var model

  signal clicked(index: int, currentText: string)

  UI.DefaultText {
    text: root.optionName

    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.verticalCenter: parent.verticalCenter
  }

  UI.MyComboBox {
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.verticalCenter: parent.verticalCenter

    currentIndex: root.currentIndex
    model: root.model

    onActivated: () => root.clicked(currentIndex, currentText)
  }
}