import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels

ListView {
  id: scenariouses
  width: parent.width
  implicitHeight: contentHeight
  property KeybindSettingsModel keybindSettings: null
  property bool showKeybinds: true

  signal scenarioRequested(string scenarioId)

  spacing: 5
  interactive: false
  clip: true

  delegate: Components.ScenarioDelegate {
    id: scenarioDelegate
    width: scenariouses.width
    keybindSettings: scenariouses.keybindSettings
    showKeybinds: scenariouses.showKeybinds
    onExecuteRequested: scenariouses.scenarioRequested(scenarioDelegate.scenario_id)
  }
}
