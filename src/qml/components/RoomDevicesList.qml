import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models 1.0

Column {
  id: room
  spacing: 8

  property var roomId: null
  property var name: ""
  property bool collapsed: false
  property var sourceModel
  property string searchQuery: ""

  Item {
    width: parent.width
    height: 20

    Text {
      id: roomTitle
      text: name
      color: themes.inactive

      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter

      font.pointSize: 14
      font.bold: true
    }

    UI.ImageButton {
      id: minimizeButton

      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter

      width: 16
      height: 16

      source: "qrc:/images/arrow.svg"

      onClicked: () => {
        collapsed = !collapsed;

        if (roomId !== null) {
          if (collapsed) {
            settingsStorage.AddToList("collapsed-rooms", roomId);
          } else {
            settingsStorage.RemoveFromList("collapsed-rooms", roomId);
          }
        }
      }

      rotation: collapsed ? 180 : 0

      Behavior on rotation {
        NumberAnimation { duration: 150 }
      }
    }
  }

  UI.DefaultText {
    text: "В этой комнате нет устройств!"

    opacity: (!collapsed && room.sourceModel.count === 0) ? 1 : 0
    height: (!collapsed && room.sourceModel.count === 0) ? implicitHeight : 0

    Behavior on opacity {
      NumberAnimation { duration: 150 }
    }

    Behavior on height {
      NumberAnimation { duration: 150 }
    }

    visible:  height > 0 && room.sourceModel.count === 0
  }

  ListView {
    id: devicesList
    clip: true
    interactive: false
    spacing: 8

    width: parent.width
    // height: contentHeight
    height: collapsed ? 0 : contentHeight

    Behavior on height {
      NumberAnimation { duration: 150 }
    }

    model: room.sourceModel

    delegate: Components.DeviceDelegate {
      width: devicesList.width
    }
  }
}