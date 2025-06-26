import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
  id: root
  width: 32
  height: 12

  property bool checked: false

  property bool internalChecked: checked

  signal toggled(bool checked)

  onCheckedChanged: {
    internalChecked = checked
  }

  Rectangle {
    id: track
    anchors.fill: parent
    radius: height / 2
    color: internalChecked ? themes.switchActive : themes.switchInactive
    Behavior on color { ColorAnimation { duration: 200 } }
  }

  Rectangle {
    id: thumb
    width: 18
    height: 18
    radius: 12
    color: themes.controlText
    anchors.verticalCenter: parent.verticalCenter
    x: internalChecked ? root.width - width : 0
    Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
    layer.enabled: true
    layer.effect: DropShadow {
      source: thumb
      radius: 5
      samples: 16
      color: "#33000000"
      horizontalOffset: 0
      verticalOffset: 1
    }
  }

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor

    onClicked: {
      internalChecked = !internalChecked
      toggled(internalChecked)
    }
  }
}
