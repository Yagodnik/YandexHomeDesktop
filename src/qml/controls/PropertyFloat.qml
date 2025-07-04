import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  height: 64

  Rectangle {
    anchors.fill: parent
    radius: 16
    color: themes.headerBackground
  }

  Item {
    id: propertyIcon

    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.verticalCenter: parent.verticalCenter

    width: 40
    height: 40

    Rectangle {
      id: iconBackground
      anchors.fill: parent
      radius: 45
      color: Qt.rgba(233 / 255, 227 / 255, 254 / 255, 1.0)
    }

    Image {
      id: iconImage
      anchors.fill: parent
      anchors.margins: 8
      source: "qrc:/images/properties/temperature.svg"
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
      id: valueText
      font.bold: true

      text: propertyState["value"] + "%"
    }

    UI.DefaultText {
      id: propertyTitle
      font.pixelSize: 14

      text: "Уровень водыыы" + " • " + "00:11"
    }
  }
}