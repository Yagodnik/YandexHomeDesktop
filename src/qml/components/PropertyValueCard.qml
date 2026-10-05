import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: 64

  property url iconSource
  property string valueText
  property string titleText

  UI.CardSurface {
    anchors.fill: parent
  }

  Item {
    id: propertyIcon
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.verticalCenter: parent.verticalCenter
    width: 40
    height: 40

    Rectangle {
      anchors.fill: parent
      radius: 45
      color: Qt.rgba(233 / 255, 227 / 255, 254 / 255, 1.0)
    }

    Image {
      id: iconImage
      anchors.fill: parent
      anchors.margins: 8
      source: root.iconSource
    }

    ColorOverlay {
      anchors.fill: iconImage
      source: iconImage
      color: themes.accent
    }
  }

  Column {
    spacing: 0
    anchors.left: propertyIcon.right
    anchors.leftMargin: 8
    anchors.verticalCenter: parent.verticalCenter

    UI.DefaultText {
      font.bold: true
      text: root.valueText
    }

    UI.DefaultText {
      font.pixelSize: 14
      text: root.titleText
    }
  }
}
