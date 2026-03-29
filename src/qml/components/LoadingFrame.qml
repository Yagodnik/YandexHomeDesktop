import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  property bool showCondition

  UI.MyProgressIndicator {
    id: loadingProgress

    anchors.centerIn: parent
    width: 30
    height: 30
    strokeWidth: 2
    opacity: showCondition ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }
  }
}