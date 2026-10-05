import QtQuick

Item {
  id: root
  property int padding: 16
  default property alias content: contentPane.data
  clip: true

  data: Item {
    id: contentPane
    anchors.fill: parent
    anchors.margins: root.padding
  }
}
