import QtQuick
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root
  property var sourceModel: null
  property var devicesModel: null
  property var favoritesModel: null
  property string favoritesTitle
  property bool favoritesCollapsed: false
  property bool preferencesAvailable: false
  signal collapseRequested(string roomId, bool collapsed)
  signal favoritesCollapseRequested(bool collapsed)
  signal deviceRequested(string deviceId)
  signal favoriteRequested(string deviceId, bool favorite)

  UI.ScrollColumn {
    id: roomsList
    objectName: "roomsList"
    anchors.fill: parent
    spacing: 8
    ScrollBar.vertical: scrollBar

    Components.FavoriteDevicesList {
      objectName: "favoritesSection"
      width: roomsList.width
      sourceModel: root.favoritesModel
      title: root.favoritesTitle
      collapsed: root.favoritesCollapsed
      preferencesAvailable: root.preferencesAvailable
      onCollapseToggled: function(collapsed) { root.favoritesCollapseRequested(collapsed); }
      onDeviceRequested: function(id) { root.deviceRequested(id); }
      onFavoriteRequested: function(id, favorite) { root.favoriteRequested(id, favorite); }
    }

    Repeater {
      model: root.sourceModel
      delegate: Components.RoomDevicesList {
        width: roomsList.width
        devicesModel: root.devicesModel
        preferencesAvailable: root.preferencesAvailable
        onCollapseRequested: function(id, collapsed) { root.collapseRequested(id, collapsed); }
        onDeviceRequested: function(id) { root.deviceRequested(id); }
        onFavoriteRequested: function(id, favorite) { root.favoriteRequested(id, favorite); }
      }
    }
  }

  UI.ListScrollBar {
    id: scrollBar
    view: roomsList
  }
}
