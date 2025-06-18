import QtQuick
import QtQuick.Controls

Item {
  id: root

  property alias text: label.text
  signal clicked()

  property var horizontalMargin: 20
  property var verticalMargin: 10

  implicitWidth: label.implicitWidth + 2 * horizontalMargin
  implicitHeight: label.implicitHeight + 2 * verticalMargin

  property color normalColor: themes.GetAccent()
  property color pressedColor: themes.GetAccent2()

  Rectangle {
    id: background
    anchors.fill: parent
    radius: 8
    color: normalColor

    Behavior on color {
      ColorAnimation {
        duration: 100
      }
    }
  }

  Text {
    id: label
    anchors.centerIn: parent
    color: themes.GetControlText()
    font.pointSize: 16
  }

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor
    onClicked: root.clicked()

    onPressed: background.color = pressedColor
    onReleased: background.color = normalColor
    onCanceled: background.color = normalColor
  }
}
