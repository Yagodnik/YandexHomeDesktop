import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

ListView {
  id: scenariouses
  width: parent.width
  implicitHeight: contentHeight

  signal scenarioRequested(string scenarioId)

  spacing: 5
  interactive: false
  clip: true

  delegate: Components.ScenarioDelegate {
    id: scenarioDelegate
    width: scenariouses.width
    onExecuteRequested: scenariouses.scenarioRequested(scenarioDelegate.scenario_id)
  }
}
