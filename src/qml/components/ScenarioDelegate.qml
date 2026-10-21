import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels

Item {
  id: delegateItem
  height: 48
  objectName: "scenarioDelegate_" + scenario_id
  required property string scenario_id
  required property string name
  required property bool is_active
  required property bool is_waiting_response
  property KeybindSettingsModel keybindSettings: null
  property bool showKeybinds: true
  signal executeRequested()

  opacity: 0

  Behavior on opacity {
    NumberAnimation {
      duration: 300
      easing.type: Easing.InOutQuad
    }
  }

  Component.onCompleted: {
    opacity = 1
  }

  Rectangle {
    id: darkOverlay
    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.1)
    visible: !delegateItem.is_active
    radius: 16
    z: 100

    MouseArea {
      anchors.fill: parent
      enabled: true
      hoverEnabled: true
      preventStealing: true
    }

    Behavior on opacity {
      NumberAnimation { duration: 150 }
    }

    opacity: visible ? 1 : 0
  }

  UI.Shadow {
    anchors.fill: background
    source: background
    horizontalOffset: 0
    verticalOffset: 2
    radius: 6
    samples: 16
    // color: Qt.rgba(0, 32 / 255, 128 / 255, 0.04)
    color: themes.shadowColor
  }

  UI.CardSurface {
    id: background
    anchors.fill: parent
  }

  UI.DefaultText {
    objectName: "scenarioName"
    text: delegateItem.name
    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 16
    anchors.right: shortcut.visible ? shortcut.left : startButton.left
    anchors.rightMargin: 8
    elide: Text.ElideRight
  }

  Components.CapabilityKeybind {
    id: shortcut
    objectName: "scenarioKeybind"
    anchors.verticalCenter: parent.verticalCenter
    anchors.right: startButton.left
    anchors.rightMargin: 8
    width: Math.min(implicitWidth, delegateItem.width * 0.45)
    visible: delegateItem.showKeybinds && delegateItem.keybindSettings !== null
    z: 101 // Inactive scenarios still allow editing their assignment.
    compact: true
    capabilityTitle: delegateItem.name
    bindingDescription: qsTr("Запуск сценария")
    deviceName: delegateItem.name
    keybindSettings: delegateItem.keybindSettings
    bindingTarget: ({ scenario_id: delegateItem.scenario_id, device_id: "", capability_type: "",
                      instance: "", action: "run", value: "" })
  }

  UI.MyProgressIndicator {
    width: 30
    height: 30
    strokeWidth: 2
    anchors.right: parent.right
    anchors.rightMargin: 9
    anchors.verticalCenter: parent.verticalCenter
    opacity: delegateItem.is_waiting_response ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }
  }

  Image {
    id: startButton
    objectName: "scenarioStart"
    source: "qrc:/images/play.svg"

    opacity: delegateItem.is_waiting_response ? 0 : 1
    visible: opacity > 0

    Behavior on opacity {
      NumberAnimation {
        duration: 100
        easing.type: Easing.InOutQuad
      }
    }

    antialiasing: true
    layer.enabled: true
    layer.smooth: true
    layer.samples: 8

    fillMode: Image.PreserveAspectFit
    scale: 0.5

    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: 8

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      enabled: delegateItem.is_active && !delegateItem.is_waiting_response

      onClicked: delegateItem.executeRequested()
    }
  }
}
