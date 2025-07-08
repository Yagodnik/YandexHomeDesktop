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

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor

    onClicked: {
      console.log("Device Id: ", deviceId, " Room Id: ", deviceRoomId, " Household Id: ", deviceHouseholdId)

      capabilitiesModel.LoadDevice(deviceId);
      // deviceDataController.RequestData(deviceId);

      router.navigateTo("device");
    }
  }
}