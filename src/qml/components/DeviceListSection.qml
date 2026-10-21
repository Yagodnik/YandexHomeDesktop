import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root
  required property string title
  property string sectionId
  property string listObjectName
  property string emptyMessage
  property var sourceModel: null
  property bool collapsed: false
  property bool preferencesAvailable: false
  property bool animate: false
  signal collapseToggled(bool collapsed)
  signal deviceRequested(string deviceId)
  signal favoriteRequested(string deviceId, bool favorite)
  implicitHeight: heading.height + body.height
  height: implicitHeight
  Component.onCompleted: animate = true

  Components.CollapsibleSectionHeader {
    id: heading
    objectName: "collapse-" + root.sectionId
    width: parent.width
    title: root.title
    collapsed: root.collapsed
    enabled: root.preferencesAvailable
    onToggleRequested: root.collapseToggled(!root.collapsed)
  }

  Item {
    id: body
    objectName: "sectionBody-" + root.sectionId
    anchors.top: heading.bottom
    width: parent.width
    readonly property real expandedHeight: content.implicitHeight + 8
    height: root.collapsed ? 0 : expandedHeight
    opacity: root.collapsed ? 0 : 1
    visible: height > 0
    enabled: !root.collapsed
    clip: true

    Behavior on height {
      enabled: root.animate
      NumberAnimation { duration: 220; easing.type: Easing.InOutCubic }
    }
    Behavior on opacity {
      enabled: root.animate
      NumberAnimation { duration: 220; easing.type: Easing.InOutCubic }
    }

    Column {
      id: content
      width: parent.width
      y: 8

      UI.DefaultText {
        objectName: "sectionEmpty-" + root.sectionId
        width: parent.width
        text: root.emptyMessage
        wrapMode: Text.WordWrap
        visible: devicesList.count === 0
      }

      Column {
        id: devicesList
        objectName: root.listObjectName
        width: parent.width
        spacing: 8
        readonly property int count: devicesRepeater.count

        Repeater {
          id: devicesRepeater
          model: root.sourceModel
          delegate: Components.DeviceDelegate {
            width: devicesList.width
            favoritesEnabled: root.preferencesAvailable
            onDeviceRequested: function(id) { root.deviceRequested(id); }
            onFavoriteRequested: function(id, favorite) { root.favoriteRequested(id, favorite); }
          }
        }
      }
    }
  }
}
