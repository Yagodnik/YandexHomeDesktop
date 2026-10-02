import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 48
  z: 200

  property string title
  property bool online: true

  signal backClicked()

  Rectangle {
    id: headerBackground
    anchors.fill: parent
    color: themes.headerBackground
  }

  DropShadow {
    anchors.fill: headerBackground
    source: headerBackground
    radius: 12
    samples: 16
    horizontalOffset: 0
    verticalOffset: 2
    color: themes.shadowColor
  }

  Image {
    id: backButton
    source: "qrc:/images/back.svg"

    antialiasing: true
    layer.enabled: true
    layer.smooth: true
    layer.samples: 8
    fillMode: Image.PreserveAspectFit

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 12

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      onClicked: root.backClicked()
    }
  }

  UI.DefaultText {
    id: deviceTitleText
    text: root.title
    anchors.verticalCenter: parent.verticalCenter
    anchors.horizontalCenter: parent.horizontalCenter
    color: themes.controlText
  }

  Image {
    id: offlineIcon
    anchors.left: deviceTitleText.right
    anchors.leftMargin: 4
    anchors.verticalCenter: deviceTitleText.verticalCenter
    source: "qrc:/images/warning.svg"
    visible: !root.online

    ToolTip.visible: offlineHover.containsMouse
    ToolTip.delay: 300
    ToolTip.text: qsTr("Устройство оффлайн!")

    MouseArea {
      id: offlineHover
      anchors.fill: parent
      hoverEnabled: true
    }
  }
}
