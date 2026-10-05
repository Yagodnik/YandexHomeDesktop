import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models 1.0

Column {
  id: room
  spacing: 8
  required property var devicesModel
  required property string name
  required property string roomId
  required property string householdId

  Text {
    id: roomTitle
    text: room.name
    color: themes.inactive

    font.pointSize: 14
    font.bold: true
  }

  DevicesFilterModel {
    id: filteredModel
    sourceModel: room.devicesModel
    householdId: room.householdId
    roomId: room.roomId
  }

  UI.DefaultText {
    text: qsTr("В этой комнате нет устройств!")

    visible: filteredModel.count === 0
  }

  ListView {
    id: devicesList
    clip: true
    interactive: false
    spacing: 8

    width: parent.width
    height: contentHeight

    model: filteredModel

    delegate: Components.DeviceDelegate {
      width: devicesList.width
    }
  }
}
