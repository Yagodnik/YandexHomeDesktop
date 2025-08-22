import QtQuick 2.15

Rectangle {
  id: skeleton
  implicitWidth: 200
  implicitHeight: 16

  property color baseColor: themes.background
  property real cornerRadius: height / 2

  radius: cornerRadius
  color: baseColor

  SequentialAnimation on opacity {
    loops: Animation.Infinite
    NumberAnimation { from: 0.6; to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
    NumberAnimation { from: 1.0; to: 0.6; duration: 600; easing.type: Easing.InOutQuad }
  }
}
