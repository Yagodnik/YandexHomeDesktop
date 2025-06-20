import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI

Slider {
  id: root

  property color trackColor: themes.GetTrackColor()
  property color fillColor: themes.GetAccent()
  property color handleColor: themes.GetAccent2()
  property color handleBorderColor: themes.GetBackground()

  implicitWidth: 200
  implicitHeight: 40

  from: 0
  to: 100
  stepSize: 1
  live: true

  signal sliderValueChanged(real value)
  onValueChanged: sliderValueChanged(value)

  background: Rectangle {
    id: groove
    anchors.verticalCenter: parent.verticalCenter
    height: 6
    radius: 3
    color: root.trackColor
    width: parent.width

    Rectangle {
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      width: root.position * parent.width
      height: parent.height
      radius: parent.radius
      color: root.fillColor

      Behavior on width {
        NumberAnimation {
          duration: 150
          easing.type: Easing.OutCubic
        }
      }
    }
  }

  handle: Item {}
}
