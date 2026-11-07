import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  objectName: "device-" + deviceId
  required property string deviceId
  required property string deviceType
  required property string name
  required property bool isFavorite
  property bool favoritesEnabled: false
  signal deviceRequested(string deviceId)
  signal favoriteRequested(string deviceId, bool favorite)
  height: 64

  opacity: 0

  Behavior on opacity {
    NumberAnimation {
      duration: 300
      easing.type: Easing.InOutQuad
    }
  }

  Component.onCompleted: {
    opacity = 1
  }

  UI.CardSurface {
    anchors.fill: parent
  }

  Image {
    id: icon

    source: deviceIcons.GetIcon(deviceType)
    width: 40
    height: 40

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 8
  }

  UI.DefaultText {
    id: nameText

    text: name

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: icon.right
    anchors.leftMargin: 12
    anchors.right: favoriteButton.left
    anchors.rightMargin: 4
    elide: Text.ElideRight
  }

  MouseArea {
    anchors.left: parent.left
    anchors.right: favoriteButton.left
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    cursorShape: Qt.PointingHandCursor

    onClicked: root.deviceRequested(root.deviceId)
  }

  UI.FavoriteButton {
    id: favoriteButton
    objectName: "favorite-" + root.deviceId
    anchors.right: parent.right
    anchors.rightMargin: 6
    anchors.verticalCenter: parent.verticalCenter
    enabled: root.favoritesEnabled
    favorite: root.isFavorite
    onClicked: root.favoriteRequested(root.deviceId, !root.isFavorite)
  }
}
