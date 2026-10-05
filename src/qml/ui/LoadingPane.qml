import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root

  property bool active: false
  property int indicatorSize: 30
  property real strokeWidth: 2
  property string message

  UI.MyProgressIndicator {
    id: indicator
    anchors.centerIn: parent
    width: root.indicatorSize
    height: root.indicatorSize
    strokeWidth: root.strokeWidth
    opacity: root.active ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }
  }

  UI.DefaultText {
    text: root.message
    visible: root.active && root.message.length !== 0
    color: themes.accent
    anchors.top: indicator.bottom
    anchors.topMargin: 15
    anchors.horizontalCenter: parent.horizontalCenter
  }
}
