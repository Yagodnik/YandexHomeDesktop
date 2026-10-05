import QtQuick
import YandexHomeDesktop.Ui as UI

Column {
  id: root
  spacing: 4

  property string title
  property var sourceModel: null

  visible: sourceModel !== null && sourceModel.count !== 0

  UI.HeadingText {
    text: root.title
    x: 16
  }

  Column {
    id: attributes
    width: root.width - 32
    x: 16
    spacing: 10

    Repeater {
      model: root.sourceModel

      delegate: Loader {
        source: delegateSource
        width: parent.width
      }
    }
  }
}
