import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels as ViewModels

UI.PageSurface {
  id: root

  required property ViewModels.ScenariosViewModel viewModel

  Component.onCompleted: viewModel.EnsureLoaded()

  UI.ErrorDialog {
    id: scenarioErrorDialog
    objectName: "scenarioErrorDialog"

    dialogTitle: qsTr("Ошибка")
    dialogMessage: qsTr("Не удалось выполнить сценарий")
  }

  Connections {
    target: root.viewModel

    function onExecutionFailed(message) {
      scenarioErrorDialog.dialogMessage = message;
      scenarioErrorDialog.openDialog();
    }
  }

  UI.RefreshHeader {
    id: heading
    width: parent.width
    title: qsTr("Все сценарии")

    objectName: "scenarioRefreshButton"
    enabled: !root.viewModel.loading

    onRefreshClicked: {
      root.viewModel.Refresh();
      heading.rotationAngle += 360;
    }
  }

  UI.PageStates {
    id: scenariosStack
    objectName: "scenariosStack"

    width: parent.width
    anchors.top: heading.bottom
    anchors.topMargin: 4
    anchors.bottom: parent.bottom

    currentIndex: root.viewModel.state === ViewModels.ScenariosViewModel.Ready ? 2
                : root.viewModel.state === ViewModels.ScenariosViewModel.Error ? 1 : 0

    UI.LoadingPane {
      active: scenariosStack.currentIndex === 0
    }

    UI.LoadErrorPane {
      active: scenariosStack.currentIndex === 1
      message: qsTr("Что-то пошло не так!")
    }

    Components.ScenariosPane {
      sourceModel: root.viewModel.scenarios
      emptyMessage: qsTr("Пока что у вас нет сценариев")
      onScenarioRequested: function(scenarioId) { root.viewModel.ExecuteScenario(scenarioId); }
    }
  }
}
