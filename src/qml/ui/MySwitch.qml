import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
  id: root
  width: 32
  height: 12

  property var checked: false
  signal toggled(bool checked)

  Rectangle {
    id: track
    anchors.fill: parent
    radius: height / 2
    color: checked ?
      themes.GetSwitchActive() : themes.GetSwitchInactive()

    Behavior on color { ColorAnimation { duration: 200 } }
  }

  Rectangle {
    id: thumb
    width: 18
    height: 18
    radius: 12
    color: themes.GetControlText()
    anchors.verticalCenter: parent.verticalCenter
    x: checked ? root.width - width : 0

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
      checked = !checked
      toggled(checked)
    }
  }
}
