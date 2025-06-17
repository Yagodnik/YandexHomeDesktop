import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  id: userDetails
  width: parent.width
  height: 56

  property bool loading: true

  Rectangle {
    id: background
    anchors.fill: parent
    color: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 1)
    radius: 16
  }

  DropShadow {
    anchors.fill: background
    source: background
    horizontalOffset: 0
    verticalOffset: 2
    radius: 6
    samples: 16
    color: Qt.rgba(0, 32 / 255, 128 / 255, 0.04)
  }

  Rectangle {
    id: avatar
    width: 32
    height: 32
    radius: 45
    color: userDetails.loading ? "#e0e0e0" : "red"

    anchors.left: parent.left
    anchors.leftMargin: 5
    anchors.verticalCenter: parent.verticalCenter

    layer.enabled: userDetails.loading
    layer.smooth: true
    layer.effect: ShaderEffect {
      property real t: shaderParams.t
      fragmentShader: "qrc:/shaders/highlight.frag.qsb"
    }
  }

  Column {
    id: details
    anchors.left: avatar.right
    anchors.leftMargin: 6
    anchors.verticalCenter: parent.verticalCenter
    spacing: 4

    Rectangle {
      width: userDetails.loading ? 120 : 0
      height: userDetails.loading ? 14 : 0
      visible: userDetails.loading
      color: "#e0e0e0"
      radius: 16

      layer.enabled: userDetails.loading
      layer.smooth: true
      layer.effect: ShaderEffect {
        property real t: shaderParams.t
        fragmentShader: "qrc:/shaders/highlight.frag.qsb"
      }
    }

    Rectangle {
      width: userDetails.loading ? 180 : 0
      height: userDetails.loading ? 12 : 0
      visible: userDetails.loading
      color: "#e0e0e0"
      radius: 16

      layer.enabled: userDetails.loading
      layer.smooth: true
      layer.effect: ShaderEffect {
        property real t: shaderParams.t
        fragmentShader: "qrc:/shaders/highlight.frag.qsb"
      }
    }

    UI.DefaultText {
      visible: !userDetails.loading
      text: "Artem Yagodnik"
    }

    UI.SubheadingText {
      visible: !userDetails.loading
      text: "YagodnikArtem2@yandex.ru"
    }
  }

  Image {
    id: logoutButton
    source: "qrc:/images/logout.svg"
    // visible: !userDetails.loading

    antialiasing: true
    layer.enabled: true
    layer.smooth: true
    layer.samples: 8

    fillMode: Image.PreserveAspectFit
    scale: 0.6
    anchors.right: parent.right
    anchors.rightMargin: 5
    anchors.verticalCenter: parent.verticalCenter

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      onClicked: {
        authorizationService.Logout();

        console.log("Logout...")
      }
    }
  }
}