import QtQuick
import QtQuick.Controls

TextField {
  color: themes.mainText
  font.pixelSize: 16

  property bool invertedColors: false

  background: Rectangle {
    color: invertedColors ? themes.background : themes.headerBackground
    radius: 8
  }
}