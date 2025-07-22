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

  DevicesFilterModel {
    id: filteredModel
    sourceModel: devicesModel
    householdId: model.householdId
    roomId: model.roomId
  }

  UI.DefaultText {
    text: "В этой комнате нет устройств!"

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