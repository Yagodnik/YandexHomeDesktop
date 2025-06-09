import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

Item {
  id: spinner
  width: 40
  height: 40

  property color trackColor: Qt.rgba(0, 0, 0, 0.1)
  property color arcColor: Qt.rgba(104 / 255, 57 / 255, 207 / 255, 1)
  property real strokeWidth: 4
  property real arcSpan: 90

  Shape {
    anchors.fill: parent
    antialiasing: true

    layer.enabled: true
    layer.smooth: true
    layer.samples: 8

    ShapePath {
      strokeWidth: spinner.strokeWidth
      strokeColor: spinner.trackColor
      fillColor: "transparent"
      capStyle: ShapePath.RoundCap

      startX: width / 2
      startY: height / 2

      PathAngleArc {
        centerX: width / 2
        centerY: height / 2
        radiusX: (width - strokeWidth) / 2
        radiusY: (height - strokeWidth) / 2
        startAngle: 0
        sweepAngle: 360
      }
    }
  }

  Item {
    id: rotatingArc
    anchors.fill: parent
    transform: Rotation {
      id: rot
      origin.x: spinner.width / 2
      origin.y: spinner.height / 2
      angle: 0

      NumberAnimation on angle {
        from: 0
        to: 360
        duration: 1000
        loops: Animation.Infinite
        easing.type: Easing.InOutQuint
      }
    }

    Shape {
      anchors.fill: parent
      antialiasing: true

      layer.enabled: true
      layer.smooth: true
      layer.samples: 8

      ShapePath {
        strokeWidth: spinner.strokeWidth
        strokeColor: spinner.arcColor
        fillColor: "transparent"
        capStyle: ShapePath.RoundCap

        startX: width / 2
        startY: height / 2

        PathAngleArc {
          centerX: width / 2
          centerY: height / 2
          radiusX: (width - strokeWidth) / 2
          radiusY: (height - strokeWidth) / 2
          startAngle: 0
          sweepAngle: spinner.arcSpan
        }
      }
    }
  }
}
