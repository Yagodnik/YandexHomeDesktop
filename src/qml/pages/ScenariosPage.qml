import QtQuick
import QtQuick.Layouts 2.15
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root

  Component.onCompleted: {
    isLoading = true;
    scenariosModel.RequestData();
  }

  property var isLoading: false

  Connections {
    target: scenariosModel

    function onDataLoadingFailed() {
      root.isLoading = false;
      console.log("Scenarios Model: Data Load - FAIL");
      scenariosStack.currentIndex = 1;
    }

    function onDataLoaded() {
      root.isLoading = false;
      console.log("Scenarios Model: Data Load - OK");
      scenariosStack.currentIndex = 2;
    }
  }

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
          if (root.isLoading) {
            return;
          }

          root.isLoading = true;
          scenariosStack.currentIndex = 0;
          scenariosModel.RequestData();

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

  StackLayout {
    id: scenariosStack

    width: parent.width
    anchors.top: heading.bottom
    anchors.topMargin: 4
    anchors.bottom: parent.bottom

    currentIndex: 0

    Item {
      UI.MyProgressIndicator {
        id: loadingProgress

        anchors.centerIn: parent
        width: 30
        height: 30
        strokeWidth: 2
        opacity: (scenariosStack.currentIndex === 0) ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
          NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
          }
        }
      }
    }

    Item {
      opacity: (scenariosStack.currentIndex === 1) ? 1 : 0
      visible: opacity > 0

      Behavior on opacity {
        NumberAnimation {
          duration: 200
          easing.type: Easing.InOutQuad
        }
      }

      Column {
        id: errorMessage
        anchors.centerIn: parent
        spacing: 15

        UI.DefaultText {
          text: qsTr("Что-то пошло не так!")
          color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1)

          anchors.horizontalCenter: errorMessage.horizontalCenter
        }
      }
    }

    Item {
      clip: true

      Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: scenariosList.implicitHeight

        interactive: true
        flickableDirection: Flickable.VerticalFlick

        Components.ScenariosList {
          id: scenariosList
        }
      }
    }
  }
}
