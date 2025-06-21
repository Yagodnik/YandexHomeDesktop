import QtQuick

Flow {
  id: content
  spacing: 8

  property Component delegate
  property var count
  property int minItemWidth: 74
  property int maxItemWidth: 100

  Repeater {
    model: content.count

    delegate: Item {
      readonly property int usableWidth: content.width
      readonly property int idealColumns: Math.floor((usableWidth + content.spacing) / (content.minItemWidth + content.spacing))
      readonly property int columns: Math.max(1, idealColumns)
      readonly property int totalSpacing: (columns - 1) * content.spacing
      readonly property int itemWidth: Math.floor((usableWidth - totalSpacing) / columns)

      width: Math.max(content.minItemWidth, Math.min(itemWidth, content.maxItemWidth))
      height: 85

      Loader {
        anchors.fill: parent
        sourceComponent: content.delegate
      }
    }
  }
}