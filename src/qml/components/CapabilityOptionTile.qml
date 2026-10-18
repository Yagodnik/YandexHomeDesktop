import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels

Item {
  id: root
  property string title
  property bool showSwatch: false
  property color swatchColor: "transparent"
  property url imageSource
  property bool showKeybinds: false
  property KeybindSettingsModel keybindSettings: null
  property var bindingTarget: ({})
  property string deviceName
  signal selected()

  implicitHeight: 90 + (showKeybinds ? 22 : 0)

  UI.CardSurface {
    id: card
    width: parent.width
    height: 90
    radius: 12

    Column {
      anchors.centerIn: parent
      width: parent.width - 12
      spacing: 6

      Rectangle {
        visible: root.showSwatch
        width: 32
        height: 32
        radius: 16
        color: root.swatchColor
        anchors.horizontalCenter: parent.horizontalCenter
      }

      Image {
        visible: !root.showSwatch
        width: 32
        height: 32
        anchors.horizontalCenter: parent.horizontalCenter
        source: root.imageSource
        onStatusChanged: {
          if (status === Image.Error) source = "qrc:/images/modes/unknown.svg";
        }
      }

      UI.DefaultText {
        width: parent.width
        text: root.title
        font.pixelSize: 10
        color: themes.controlText
        wrapMode: Text.WordWrap
        maximumLineCount: 3
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
      }
    }

    MouseArea {
      objectName: "optionAction"
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      onClicked: root.selected()
    }
  }

  Components.CapabilityKeybind {
    objectName: "optionKeybind"
    anchors.right: parent.right
    anchors.rightMargin: 4
    y: card.height - 4
    width: Math.min(implicitWidth, parent.width - 8)
    visible: root.showKeybinds
    compact: true
    capabilityTitle: root.title
    keybindSettings: root.keybindSettings
    bindingTarget: root.bindingTarget
    deviceName: root.deviceName
  }
}
