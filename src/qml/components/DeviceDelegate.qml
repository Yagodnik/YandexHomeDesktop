import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  width: parent.width
  height: 64

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
    radius: 16
  }

  Image {
    id: icon

    source: "qrc:/images/light.svg"
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

      deviceModel.RequestData(deviceId);

      router.navigateTo("device");
    }
  }
}