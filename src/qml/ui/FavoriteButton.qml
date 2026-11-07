import QtQuick
import QtQuick.Controls

AbstractButton {
  id: root
  implicitWidth: 44
  implicitHeight: 44
  property bool favorite: false
  text: favorite ? qsTr("Удалить из избранного") : qsTr("Добавить в избранное")
  Accessible.name: text
  hoverEnabled: true
  HoverHandler { cursorShape: Qt.PointingHandCursor }

  background: Rectangle {
    radius: 8
    property color accentColor: themes.accent
    color: Qt.rgba(accentColor.r, accentColor.g, accentColor.b, root.down ? 0.2 : root.hovered ? 0.1 : 0)
    border.width: root.visualFocus ? 2 : 0
    border.color: themes.accent
  }

  contentItem: Canvas {
    id: star
    property color starColor: root.favorite ? themes.accent : themes.inactive
    property bool filled: root.favorite
    onStarColorChanged: requestPaint()
    onFilledChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
      const ctx = getContext("2d");
      ctx.clearRect(0, 0, width, height);
      ctx.beginPath();
      for (let i = 0; i < 10; ++i) {
        const angle = -Math.PI / 2 + i * Math.PI / 5;
        const radius = i % 2 === 0 ? 9 : 4.125;
        const x = width / 2 + Math.cos(angle) * radius;
        const y = height / 2 + Math.sin(angle) * radius;
        if (i === 0) { ctx.moveTo(x, y); } else { ctx.lineTo(x, y); }
      }
      ctx.closePath();
      ctx.lineWidth = 1.5;
      ctx.lineJoin = "round";
      ctx.strokeStyle = starColor;
      ctx.stroke();
      if (filled) {
        ctx.fillStyle = starColor;
        ctx.fill();
      }
    }
  }

  ToolTip.visible: hovered
  ToolTip.delay: 500
  ToolTip.text: text
}
