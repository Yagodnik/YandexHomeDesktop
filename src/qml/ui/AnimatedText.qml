import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
  id: root
  property alias text: label.text
  property color color: "black"
  property var pointSize: 20

  width: label.implicitWidth
  height: label.implicitHeight

  Text {
    id: label
    anchors.horizontalCenter: parent.horizontalCenter
    y: root.height
    opacity: 0
    color: root.color
    font.pointSize: root.pointSize
    horizontalAlignment: Text.AlignHCenter
  }

  Component.onCompleted: animation.running = true

  SequentialAnimation {
    id: animation
    running: false

    ParallelAnimation {
      NumberAnimation {
        target: label;
        property: "opacity";
        from: 0; to: 1;
        duration: 600;
        easing.type: Easing.InOutQuad
      }

      NumberAnimation {
        target: label;
        property: "y";
        from: -root.height;
        to: root.height - label.implicitHeight;
        duration: 600;
        easing.type: Easing.OutQuad
      }
    }

    // PauseAnimation { duration: 1000 }
    //
    // ParallelAnimation {
    //   NumberAnimation { target: label; property: "opacity"; from: 1; to: 0; duration: 600; easing.type: Easing.InOutQuad }
    //   NumberAnimation { target: label; property: "y"; from: root.height - 20; to: root.height; duration: 600; easing.type: Easing.InQuad }
    // }
  }
}
