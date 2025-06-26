import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models 1.0

Column {
  id: room
  spacing: 8

  Text {
    id: roomTitle
    text: name
    color: themes.inactive

    font.pointSize: 14
    font.bold: true
  }

  ListView {
    id: devicesList
    clip: true
    interactive: false
    spacing: 8

    width: parent.width
    height: contentHeight

    model: DevicesFilterModel {
      sourceModel: devicesModel
      householdId: model.householdId
      roomId: model.roomId
    }

    delegate: Components.DeviceDelegate {
      width: devicesList.width
    }
  }
}