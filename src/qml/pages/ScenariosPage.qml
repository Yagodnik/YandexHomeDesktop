import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
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

  property bool isLoading: false

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

  UI.PageStates {
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

    Components.ScenariosPane {
      sourceModel: scenariosModel
      emptyMessage: qsTr("Пока что у вас нет сценариев")
      onScenarioRequested: function(index) { scenariosModel.ExecuteScenario(index); }
    }
  }
}
