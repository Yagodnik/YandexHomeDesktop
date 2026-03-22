import QtQuick
import QtQuick.Layouts 2.15
import QtQuick.Dialogs
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root

  property var isLoading: false

  Component.onCompleted: {
    isLoading = true;
    scenariosModel.RequestData();
  }

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

    function onScenarioExecutionFailed() {
      scenarioErrorDialog.openDialog();
    }
  }

  UI.ErrorDialog {
    id: scenarioErrorDialog

    dialogTitle: "Ошибка"
    dialogMessage: "Не удалось выполнить сценарий"
  }

  Components.ScenariosPageHeader {
    id: header
    
    onReloadClicked: {
      if (root.isLoading) {
        return;
      }
      root.isLoading = true;

      scenariosStack.currentIndex = 0;
      scenariosModel.RequestData();
    }
  }

  StackLayout {
    id: scenariosStack

    width: parent.width
    anchors.top: header.bottom
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
          color: themes.inactive

          anchors.horizontalCenter: errorMessage.horizontalCenter
        }
      }
    }

    Item {
      clip: true

      UI.DefaultText {
        anchors.centerIn: parent
        text: "Пока что у вас нет сценариев"

        visible: scenariosModel.count === 0
      }

      Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: scenariosList.implicitHeight
        visible: scenariosModel.count !== 0

        interactive: true
        flickableDirection: Flickable.VerticalFlick

        Components.ScenariosList {
          id: scenariosList
        }
      }

      ScrollBar {
        id: scrollBar

        width: 10
        height: flickable.height
        anchors.left: flickable.right
        anchors.leftMargin: 4
        policy: flickable.contentHeight > flickable.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

        contentItem: Rectangle {
          implicitWidth: 10
          radius: 4
          color: themes.headerBackground
        }

        background: Item {}
      }
    }
  }
}
