import QtQuick
import QtQuick.Layouts 2.15
import QtQuick.Dialogs
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root

  Component.onCompleted: {
    isLoading = true;
    scenariosModel.RequestData();
  }

  UI.ErrorDialog {
    id: scenarioErrorDialog

    dialogTitle: qsTr("Ошибка")
    dialogMessage: qsTr("Не удалось выполнить сценарий")
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

    function onScenarioExecutionFailed() {
      scenarioErrorDialog.openDialog();
    }
  }

  UI.RefreshHeader {
    id: heading
    width: parent.width
    title: qsTr("Все сценарии")

    onRefreshClicked: {
      if (root.isLoading) {
        return;
      }

      root.isLoading = true;
      scenariosStack.currentIndex = 0;
      scenariosModel.RequestData();

      heading.rotationAngle += 360;
    }
  }

  StackLayout {
    id: scenariosStack

    width: parent.width
    anchors.top: heading.bottom
    anchors.topMargin: 4
    anchors.bottom: parent.bottom

    currentIndex: 0

    UI.LoadingPane {
      active: scenariosStack.currentIndex === 0
    }

    UI.LoadErrorPane {
      active: scenariosStack.currentIndex === 1
      message: qsTr("Что-то пошло не так!")
    }

    Item {
      clip: true

      UI.DefaultText {
        anchors.centerIn: parent
        text: qsTr("Пока что у вас нет сценариев")

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
