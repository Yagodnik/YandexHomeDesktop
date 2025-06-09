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

  property color normalColor: Qt.rgba(104 / 255, 57 / 255, 207 / 255, 1)
  property color pressedColor: Qt.rgba(84 / 255, 45 / 255, 168 / 255, 1)

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
    color: "white"
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
