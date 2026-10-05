import QtQuick
import QtQuick.Controls
import YandexHomeDesktop.Components as Components

Flickable {
  id: root

  property var capabilitiesSourceModel: null
  property var propertiesSourceModel: null
  property string capabilitiesTitle
  property string propertiesTitle
  property int topPadding: 4
  property int bottomPadding: 12

  contentWidth: width
  contentHeight: attributes.y + attributes.implicitHeight + bottomPadding
  clip: true
  flickableDirection: Flickable.VerticalFlick

  ScrollBar.vertical: ScrollBar {
    width: 10
    policy: root.contentHeight > root.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    opacity: hovered ? 1.0 : 0.2
    anchors.right: parent.right
    anchors.rightMargin: 3

    contentItem: Rectangle {
      implicitWidth: 10
      radius: 16
      color: themes.headerBackground
    }
  }

  Column {
    id: attributes
    width: root.width
    y: root.topPadding
    spacing: 4

    Components.DeviceAttributeSection {
      width: parent.width
      title: root.capabilitiesTitle
      sourceModel: root.capabilitiesSourceModel
    }

    Components.DeviceAttributeSection {
      width: parent.width
      title: root.propertiesTitle
      sourceModel: root.propertiesSourceModel
    }
  }
}
