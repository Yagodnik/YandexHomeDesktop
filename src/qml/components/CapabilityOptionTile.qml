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
  property bool isSelected: false
  property bool showKeybinds: false
  property KeybindSettingsModel keybindSettings: null
  property var bindingTarget: ({})
  property string deviceName
  signal selected()

  implicitHeight: card.height + (showKeybinds ? 22 : 0)
  Accessible.role: Accessible.RadioButton
  Accessible.name: title
  Accessible.checkable: true
  Accessible.checked: isSelected

  UI.CardSurface {
    id: card
    objectName: "optionCard"
    width: parent.width
    height: 84
    radius: 12
    border.width: root.isSelected ? 2 : 0
    border.color: themes.accent

    Column {
      objectName: "optionContent"
      anchors.centerIn: parent
      width: Math.max(0, parent.width - 20)
      spacing: 8

      Item {
        objectName: "optionVisual"
        anchors.horizontalCenter: parent.horizontalCenter
        width: 32
        height: 32

        Rectangle {
          visible: root.showSwatch
          anchors.fill: parent
          radius: 16
          color: root.swatchColor
        }

        Image {
          visible: !root.showSwatch
          anchors.fill: parent
          fillMode: Image.PreserveAspectFit
          source: root.imageSource
          onStatusChanged: {
            if (status === Image.Error) source = "qrc:/images/modes/unknown.svg";
          }
        }
      }

      UI.DefaultText {
        objectName: "optionTitle"
        width: parent.width
        text: root.title
        font.pixelSize: 11
        color: themes.controlText
        wrapMode: Text.Wrap
        maximumLineCount: 2
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
