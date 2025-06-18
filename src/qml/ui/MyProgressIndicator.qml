import QtQuick 2.15

Item {
  id: spinner
  width: 24
  height: 24

  // property color trackColor: Qt.rgba(0, 0, 0, 0.1)
  property color trackColor: themes.GetTrackColor()
  property color arcColor: themes.GetAccent()
  property real strokeWidth: 3
  property real arcSpan: 90

  Canvas {
    anchors.fill: parent
    antialiasing: true

    onPaint: {
      var ctx = getContext("2d");
      ctx.reset();
      var centerX = width / 2;
      var centerY = height / 2;
      var radius = (width - strokeWidth) / 2;

      ctx.clearRect(0, 0, width, height);

      ctx.beginPath();
      ctx.strokeStyle = spinner.trackColor;
      ctx.lineWidth = strokeWidth;
      ctx.lineCap = "round";
      ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
      ctx.stroke();

      // Draw arc
      ctx.beginPath();
      ctx.strokeStyle = spinner.arcColor;
      ctx.lineWidth = strokeWidth;
      ctx.lineCap = "round";

      var startAngle = -Math.PI / 2;  // start at top
      var endAngle = startAngle + (spinner.arcSpan * Math.PI / 180);

      ctx.arc(centerX, centerY, radius, startAngle, endAngle);
      ctx.stroke();
    }
  }

  RotationAnimator on rotation {
    from: 0
    to: 360
    duration: 1000
    loops: Animation.Infinite
    running: true
    easing.type: Easing.InOutQuint
    target: spinner
  }
}
