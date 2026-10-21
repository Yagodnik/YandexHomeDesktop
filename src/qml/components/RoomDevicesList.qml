import QtQuick
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models 1.0

Components.DeviceListSection {
  id: room
  required property var devicesModel
  required property string name
  required property string roomId
  required property string householdId
  required property bool isCollapsed
  signal collapseRequested(string roomId, bool collapsed)
  title: name
  sectionId: roomId
  listObjectName: "roomDevices-" + roomId
  collapsed: isCollapsed
  sourceModel: filteredModel
  emptyMessage: qsTr("В этой комнате нет устройств!")
  onCollapseToggled: function(collapsed) { room.collapseRequested(room.roomId, collapsed); }

  DevicesFilterModel {
    id: filteredModel
    sourceModel: room.devicesModel
    householdId: room.householdId
    roomId: room.roomId
  }
}
