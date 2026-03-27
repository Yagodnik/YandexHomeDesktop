import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root

  state: "closed"
  property var myVisible: false

  function open() {
    myVisible = true
    state = "opened";
  }

  function close() {
    myVisible = false
    state = "closed";
  }

  visible: true
  anchors.left: parent.left
  anchors.right: parent.right
  z: 600
  height: 400

  y: parent.height


  Behavior on y {
    NumberAnimation {
      duration: 300
      easing.type: Easing.InOutQuad
    }
  }

  Behavior on opacity {
    NumberAnimation {
      duration: 300
      easing.type: Easing.InOutQuad
    }
  }

  states: [
    State {
      name: "opened"

      PropertyChanges {
        target: root
        // visible: true
        opacity: 1
        y: parent.height - root.height
      }
    },

    State {
      name: "closed"

      PropertyChanges {
        target: root
        // visible: false
        opacity: 0
        y: parent.height
      }
    }
  ]

  Shape {
    id: background
    anchors.fill: parent

    ShapePath {
      id: shape
      fillColor: themes.headerBackground
      strokeWidth: 0

      property real w: background.width
      property real h: background.height
      property real r: 16

      startX: r
      startY: 0

      PathLine { x: shape.w - shape.r; y: 0 }
      PathQuad {
        x: shape.w
        y: shape.r
        controlX: shape.w
        controlY: 0
      }

      PathLine { x: shape.w; y: shape.h }
      PathLine { x: 0; y: shape.h }
      PathLine { x: 0; y: shape.r }
      PathQuad {
        x: shape.r
        y: 0
        controlX: 0
        controlY: 0
      }
    }
  }

  UI.HeadingText {
    id: selectTitle
    text: "Авторизация"

    font.pixelSize: 24
    font.bold: true

    anchors.left: parent.left
    anchors.leftMargin: 16

    anchors.top: parent.top
    anchors.topMargin: 12
  }

  Item {
    clip: true
    anchors.top: selectTitle.bottom
    anchors.topMargin: 8
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    StackLayout {
      id: authorizationSteps
      anchors.fill: parent

      currentIndex: 0

      Item {
        anchors.fill: parent

        UI.HeadingText {
          id: selectProviderTitle
          text: "Выберите провайдера"

          font.pixelSize: 18
          font.bold: true

          anchors.left: parent.left
          anchors.leftMargin: 16

          anchors.top: parent.top
          anchors.topMargin: 12
        }

        ListView {
          anchors.top: selectProviderTitle.bottom
          anchors.topMargin: 10
          anchors.left: parent.left
          anchors.leftMargin: 16
          anchors.right: parent.right
          anchors.rightMargin: 16
          anchors.bottom: parent.bottom

          model: ListModel {
            ListElement {
              providerName: "Yandex"
              iconPath: "qrc:/images/icon.png"
            }

            ListElement {
              providerName: "Home Assistant"
              iconPath: "qrc:/images/icon.png"
            }
          }

          delegate: Row {
            width: parent.width
            height: 48

            Image {
              id: providerIcon
              anchors.verticalCenter: parent.verticalCenter
              anchors.left: parent.left

              width: 32
              height: 32

              source: model.iconPath
            }

            UI.HeadingText {
              id: providerTitle
              anchors.verticalCenter: parent.verticalCenter
              anchors.left: providerIcon.right
              anchors.leftMargin: 12

              font.pixelSize: 16
              font.bold: false

              text: model.providerName
            }

            MouseArea {
              anchors.fill: parent
              cursorShape: Qt.PointingHandCursor

              onClicked: () => {
                console.log(model.providerName);
                authorizationSteps.currentIndex = 1;
              }
            }
          }
        }
      }

      Item {
        anchors.fill: parent

        Connections {
          target: authorizationSteps

          onCurrentIndexChanged: () => {
            if (authorizationSteps.currentIndex === 1) {
              authorizationService.AttemptAuthorization({});
            }
          }
        }

        Column {
          anchors.fill: parent
          anchors.leftMargin: 16
          anchors.rightMargin: 16

          spacing: 10

          Row {
            width: parent.width
            height: 32

            Image {
              id: providerIcon2

              anchors.verticalCenter: parent.verticalCenter
              anchors.left: parent.left

              width: 24
              height: 24

              source: "qrc:/images/icon.png"
            }

            UI.HeadingText {
              id: yandexProviderTitle
              text: "Yandex"

              font.pixelSize: 16
              font.bold: true

              anchors.left: providerIcon2.right
              anchors.leftMargin: 8
              anchors.verticalCenter: parent.verticalCenter
            }

            UI.ImageButton {
              id: minimizeButton

              anchors.right: parent.right
              anchors.verticalCenter: parent.verticalCenter

              width: 16
              height: 16
              rotation: 270

              source: "qrc:/images/arrow.svg"

              onClicked: () => {
                authorizationSteps.currentIndex = 0;
              }
            }
          }

          Column {
            width: parent.width
            spacing: 6

            UI.HeadingText {
              text: "Введите код подтверждения"

              anchors.left: parent.left

              font.pixelSize: 14
              font.bold: true
            }

            UI.MyInputField {
              id: yandexTokenInput
              width: parent.width

              invertedColors: true

              placeholderText: "Код подтверждения"
            }
          }

          Connections {
            target: authorizationService

            function onAuthorized() {
              authorizationButton.loading = false;
              console.log("New authorization");
            }

            function onAuthorizationFailed() {
              authorizationButton.loading = false;
              console.log("Authorization failed");
            }
          }

          UI.MyButton {
            id: authorizationButton
            width: parent.width
            text: "Авторизоваться"

            property bool loading: false

            onClicked: () => {
              loading = true;

              console.log("Code: " + yandexTokenInput.text);
              authorizationService.SaveAuthToken(yandexTokenInput.text);
            }

            Rectangle {
              anchors.fill: parent
              color: "transparent"
              visible: authorizationButton.loading
              z: 1

              Rectangle {
                anchors.fill: parent
                radius: 8
                color: "#80000000"
              }

              UI.MyProgressIndicator {
                anchors.centerIn: parent
                visible: authorizationButton.loading
              }
            }

            enabled: !loading
          }

          UI.DefaultText {
            id: errorText
            color: "red"

            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap

            text: ""
          }
        }
      }

      Item {
        UI.HeadingText {
          text: "3"

          font.pixelSize: 24
          font.bold: true
        }

        UI.MyButton {
          text: "Go to 0"
          onClicked: () => {
            authorizationSteps.currentIndex = 0;
          }
        }
      }
    }
  }
}