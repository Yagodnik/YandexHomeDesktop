import QtQuick
import QtQuick.Layouts

Item {
  id: root

  property int minItemWidth: 74
  property int maxItemWidth: 100
  property int spacing: 8
  property int count: 0
  property Component delegate
  property int itemHeight: 85

  property int columns: Math.max(1,
    Math.floor((width + spacing) / (minItemWidth + spacing)))

  property int rows: Math.ceil(count / columns)
  implicitHeight: (rows * itemHeight) + ((rows - 1) * spacing)

  GridLayout {
    id: grid
    anchors.fill: parent
    columns: root.columns
    rowSpacing: root.spacing
    columnSpacing: root.spacing

    Repeater {
      model: root.count

      delegate: Item {
        Layout.preferredWidth: Math.min(
          root.maxItemWidth,
          Math.floor((root.width - (root.columns - 1) * root.spacing) / root.columns)
        )
        Layout.preferredHeight: root.itemHeight

        Loader {
          anchors.fill: parent
          sourceComponent: root.delegate
        }
      }
    }
  }
}
