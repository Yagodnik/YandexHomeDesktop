import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  id: householdSelect
  height: 28

  property var currentHouseholdName

  signal clicked()

  Item {
    width: heading.width + 4 + headingIcon.width
    height: 28

    anchors.top: parent.top
    anchors.topMargin: 16

    UI.HeadingText {
      id: heading
      text: currentHouseholdName
      font.pointSize: 24
      font.bold: true
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
    }

    Image {
      id: headingIcon
      source: "qrc:/images/arrow.svg"

      anchors.left: heading.right
      anchors.leftMargin: 2

      anchors.verticalCenter: parent.verticalCenter

      width: 18
      height: 18
      rotation: 180
    }

    ColorOverlay {
      anchors.fill: headingIcon
      source: headingIcon
      color: themes.mainText
      rotation: 180
    }

    MouseArea {
      anchors.fill: parent

      onClicked: () => householdSelect.clicked()
    }
  }
}