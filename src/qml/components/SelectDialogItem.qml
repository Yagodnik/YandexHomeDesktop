import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Column {
  id: root
  spacing: 6

  property bool selected
  property int elementsCount

  signal clicked()

  Item {
    width: parent.width
    height: 32

    Image {
      id: householdIcon
      width: 24
      height: 24
      anchors.verticalCenter: parent.verticalCenter
      anchors.left: parent.left
      source: "qrc:/images/household.svg"
    }

    ColorOverlay {
      anchors.fill: householdIcon
      source: householdIcon
      color: root.selected ? themes.accent : themes.inactive
    }

    UI.DefaultText {
      id: houseName
      text: model.name
      anchors.left: householdIcon.right
      anchors.leftMargin: 12
      anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor

      onClicked: () => root.clicked()
    }
  }

  Rectangle {
    visible: model.index < root.elementsCount - 1

    height: 1
    anchors.left: parent.left
    anchors.right: parent.right
    color: themes.inactive
  }
}