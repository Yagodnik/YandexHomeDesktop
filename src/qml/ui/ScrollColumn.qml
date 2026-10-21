import QtQuick

Flickable {
  id: root
  property int spacing: 8
  property int topPadding: 0
  property int bottomPadding: 0
  default property alias content: contentColumn.data

  contentWidth: width
  contentHeight: contentColumn.y + contentColumn.implicitHeight + bottomPadding
  flickableDirection: Flickable.VerticalFlick
  clip: true

  data: Column {
    id: contentColumn
    parent: root.contentItem
    width: root.width
    y: root.topPadding
    spacing: root.spacing
  }
}
