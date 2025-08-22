import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

ListView {
  id: scenariouses
  width: parent.width
  implicitHeight: contentHeight

  model: scenariosModel

  spacing: 5
  interactive: false
  clip: true

  delegate: Components.ScenarioDelegate {
    width: scenariouses.width
  }
}