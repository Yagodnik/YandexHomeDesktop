import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
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

  Rectangle {
    anchors.fill: parent
    color: themes.headerBackground
    radius: 16
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
  }

  Image {
    id: favoriteButton
    source: "qrc:/images/star.svg"

    width: 48
    height: 48

    antialiasing: true
    layer.enabled: true
    layer.smooth: true
    layer.samples: 8

    fillMode: Image.PreserveAspectFit
    scale: 0.5

    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: 8

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor

      onClicked: {
        console.log("Adding/removing device from favorites: " + deviceId);
        const list = settingsStorage.GetList("favorite-devices");
        console.log("Current list: " + list);
        settingsStorage.SaveList("favorite-devices", [deviceId])
      }
    }
  }

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor

    onClicked: {
      console.log("Device Delegate: Device Id: ", deviceId, " Room Id: ", deviceRoomId, " Household Id: ", deviceHouseholdId)

      deviceController.LoadDevice(deviceId);

      router.navigateTo("device");
    }
  }
}