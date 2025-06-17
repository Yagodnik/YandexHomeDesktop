import QtQuick 2.15
import QtQuick.Layouts 2.15
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  UI.MyProgressIndicator {
    anchors.centerIn: parent

    property bool isLoaded: false

    id: loadingProgress
    width: 30
    height: 30
    strokeWidth: 2
    opacity: !isLoaded ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }
  }

  Flickable {
    id: flickable
    anchors.fill: parent
    contentWidth: parent.width
    contentHeight: contentItem.implicitHeight

    interactive: true
    flickableDirection: Flickable.VerticalFlick

    Column {
      id: contentItem
      width: flickable.width

      spacing: 12

      Item {
        id: heading
        width: parent.width
        height: 32

        UI.HeadingText {
          id: headingTitle
          text: "Все сценарии"
          anchors.left: parent.left
          anchors.verticalCenter: parent.verticalCenter
        }

        Image {
          id: reloadButton
          source: "qrc:/images/reload.svg"

          property real rotationAngle: 0

          antialiasing: true
          layer.enabled: true
          layer.smooth: true
          layer.samples: 8

          fillMode: Image.PreserveAspectFit
          transform: Rotation {
            id: rot
            origin.x: reloadButton.width / 2
            origin.y: reloadButton.height / 2
            angle: reloadButton.rotationAngle
          }

          anchors.verticalCenter: parent.verticalCenter
          anchors.right: parent.right
          anchors.rightMargin: 8

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor

            onClicked: {
              scenariosModel.RequestData();
              loadingProgress.isLoaded = false;

              reloadButton.rotationAngle += 360;
            }
          }

          Behavior on rotationAngle {
            NumberAnimation {
              duration: 500
              easing.type: Easing.InOutCubic
            }
          }
        }
      }

      Connections {
        target: scenariosModel

        function onDataLoaded() {
          console.log("We are done!");
          loadingProgress.isLoaded = true
        }
      }

      ListView {
        id: scenariouses
        width: parent.width
        implicitHeight: contentHeight

        model: scenariosModel

        Component.onCompleted: {
          scenariosModel.RequestData();
        }

        spacing: 5
        interactive: false
        clip: true

        delegate: Components.ScenarioDelegate {}
      }
    }
  }
}
