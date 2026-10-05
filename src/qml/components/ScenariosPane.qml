import QtQuick
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root
  property var sourceModel: null
  property string emptyMessage
  readonly property bool empty: sourceModel === null || sourceModel.count === 0
  signal scenarioRequested(string scenarioId)

  UI.DefaultText {
    anchors.centerIn: parent
    text: root.emptyMessage
    visible: root.empty
  }

  Components.ScenariosList {
    id: scenariosList
    objectName: "scenariosList"
    anchors.fill: parent
    model: root.sourceModel
    visible: !root.empty
    interactive: true
    ScrollBar.vertical: scrollBar
    onScenarioRequested: function(scenarioId) { root.scenarioRequested(scenarioId); }
  }

  UI.ListScrollBar {
    id: scrollBar
    objectName: "scenariosScrollBar"
    view: scenariosList
    visible: !root.empty
  }
}
