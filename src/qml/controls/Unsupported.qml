import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  objectName: "unsupportedCapabilityControl"
  height: Math.max(48, label.implicitHeight + 24)

  UI.CardSurface {
    anchors.fill: parent
  }

  UI.DefaultText {
    id: label
    objectName: "unsupportedCapabilityLabel"
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.right: parent.right
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter
    wrapMode: Text.Wrap

    text: qsTr("Неподдерживаемое умение: %1").arg(name)
  }
}
