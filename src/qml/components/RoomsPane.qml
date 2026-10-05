import QtQuick
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root
  property var sourceModel: null

  ListView {
    id: roomsList
    anchors.fill: parent
    clip: true
    spacing: 8
    model: root.sourceModel
    ScrollBar.vertical: scrollBar

    delegate: Components.RoomDevicesList {
      width: roomsList.width
    }
  }

  UI.ListScrollBar {
    id: scrollBar
    view: roomsList
  }
}
