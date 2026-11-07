import QtQuick
import Qt5Compat.GraphicalEffects

Item {
  id: root
  property var source
  property real radius: 6
  property int samples: 16
  property real horizontalOffset: 0
  property real verticalOffset: 2
  property color color: themes.shadowColor

  Loader {
    anchors.fill: parent
    // Shader effects require a graphics backend and are skipped during software rendering.
    active: root.GraphicsInfo.api !== GraphicsInfo.Unknown
      && root.GraphicsInfo.api !== GraphicsInfo.Software
    sourceComponent: DropShadow {
      source: root.source
      radius: root.radius
      samples: root.samples
      horizontalOffset: root.horizontalOffset
      verticalOffset: root.verticalOffset
      color: root.color
    }
  }
}
