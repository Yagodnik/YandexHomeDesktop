import QtQuick
import YandexHomeDesktop.Ui as UI

Column {
  id: root
  spacing: 4

  property string title
  property var sourceModel: null
  property int bottomSpace: 0

  visible: sourceModel !== null && sourceModel.count !== 0

  UI.HeadingText {
    text: root.title
    x: 16
  }

  ListView {
    width: root.width - 32
    x: 16
    height: contentHeight + root.bottomSpace
    clip: true
    interactive: false
    spacing: 10
    model: root.sourceModel

    delegate: Loader {
      source: delegateSource
      width: parent.width
    }
  }
}
