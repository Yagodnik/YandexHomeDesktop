import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: rangeControl
  height: 52

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
    radius: 16
  }

  Item {
    id: top

    anchors.top: rangeControl.top
    anchors.topMargin: 4

    anchors.left: parent.left
    anchors.leftMargin: 8
    anchors.right: parent.right
    anchors.rightMargin: 8

    height: 24

    UI.DefaultText {
      id: propertyTitle

      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter

      text: "Яркость"
    }

    UI.DefaultText {
      id: valueText

      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter

      text: rangeSlider.value + "%"
    }
  }

  property var brightness: 50

  UI.MySlider {
    id: rangeSlider
    anchors.top: top.bottom
    anchors.left: parent.left
    anchors.leftMargin: 8
    anchors.right: parent.right
    anchors.rightMargin: 8

    height: 20

    from: 0
    to: 100
    stepSize: 1
  }
}

