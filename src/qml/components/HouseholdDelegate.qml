import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Column {
  id: root
  spacing: 6
  property string title
  property bool selected: false
  property bool showSeparator: false
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
      text: root.title
      anchors.left: householdIcon.right
      anchors.leftMargin: 12
      anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      onClicked: root.clicked()
    }
  }

  Rectangle {
    visible: root.showSeparator
    height: 1
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: 36
    color: themes.inactive
  }
}
