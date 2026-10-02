import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root

  property bool active: false

  UI.MyProgressIndicator {
    anchors.centerIn: parent
    width: 30
    height: 30
    strokeWidth: 2
    opacity: root.active ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }
  }
}
