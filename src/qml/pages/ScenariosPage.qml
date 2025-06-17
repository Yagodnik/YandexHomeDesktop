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

      UI.HeadingText {
        id: heading
        text: "Все сценарии"
        width: parent.width
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
