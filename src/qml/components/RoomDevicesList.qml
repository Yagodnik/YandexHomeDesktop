import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models 1.0

Column {
  id: room
  spacing: 8

  property bool collapsed: false

  Row {
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

      onClicked: collapsed = !collapsed

      rotation: collapsed ? 180 : 0

      Behavior on rotation {
        NumberAnimation { duration: 150 }
      }
    }
  }

  DevicesFilterModel {
    id: filteredModel
    sourceModel: devicesModel
    householdId: model.householdId
    roomId: model.roomId
  }

  UI.DefaultText {
    text: "В этой комнате нет устройств!"

    opacity: (!collapsed && filteredModel.count === 0) ? 1 : 0
    height: (!collapsed && filteredModel.count === 0) ? implicitHeight : 0

    Behavior on opacity {
      NumberAnimation { duration: 150 }
    }

    Behavior on height {
      NumberAnimation { duration: 150 }
    }

    visible:  height > 0 && filteredModel.count === 0
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

    model: filteredModel

    delegate: Components.DeviceDelegate {
      width: devicesList.width
    }
  }
}