import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Models as Models
import YandexHomeDesktop.Capabilities as Capabilities
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels

Item {
  id: root
  height: 48
  property bool showKeybinds: false
  property KeybindSettingsModel keybindSettings: null
  property var keybindTarget: ({})
  property string keybindDeviceName

  property var deviceModelData: model

  Capabilities.Modes {
    id: modesCapability

    state: model.attributeState
    parameters: model.attributeParameters
    titlesList: iotTitles
  }

  UI.CardSurface {
    anchors.fill: parent
  }

  UI.DefaultText {
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.right: comboBox.left
    anchors.rightMargin: 8
    anchors.verticalCenter: parent.verticalCenter

    text: modesCapability.title
    elide: Text.ElideRight
  }

  UI.MyComboBox {
    id: comboBox
    objectName: "deviceModePicker"
    width: 120
    popupWidth: root.showKeybinds ? Math.min(280, root.width - 24) : width

    textRole: "displayText"

    model: Models.ModesFilterModel {
      sourceModel: modesModel

      allowedModes: modesCapability.parameters["modes"]
    }

    anchors.right: parent.right
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    property alias selectedModeId: modesCapability.value

    Component.onCompleted: {
      updateCurrentIndexFromValue()
    }

    onSelectedModeIdChanged: updateCurrentIndexFromValue()
    onCountChanged: updateCurrentIndexFromValue()

    onActivated: {
      const capability_action = modesCapability.Create(model.get(currentIndex).id);
      capabilitiesModel.UseCapability(deviceModelData.index, capability_action);
    }

    delegate: Item {
      id: option
      objectName: "modeOption-" + model.modeId
      width: comboBox.popup.width
      height: root.showKeybinds ? 42 : 32
      readonly property string modeId: model.modeId
      readonly property string title: model.displayText
      readonly property bool isSelected: modeId === modesCapability.value

      Rectangle {
        anchors.fill: parent
        radius: 8
        color: option.isSelected ? themes.accent : "transparent"
      }

      UI.DefaultText {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: root.showKeybinds ? keybind.left : parent.right
        anchors.rightMargin: root.showKeybinds ? 8 : comboBox.rightPadding
        anchors.verticalCenter: parent.verticalCenter
        text: option.title
        font: comboBox.font
        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        color: option.isSelected ? themes.controlText2 : themes.controlText
      }

      MouseArea {
        objectName: "modeOptionAction"
        anchors.fill: parent
        anchors.rightMargin: root.showKeybinds ? keybind.width + 12 : 0
        cursorShape: Qt.PointingHandCursor
        onClicked: {
          comboBox.currentIndex = index;
          comboBox.activated(index);
          comboBox.indexSelected(index);
          comboBox.popup.close();
        }
      }

      Components.CapabilityKeybind {
        id: keybind
        objectName: "modeOptionKeybind"
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(implicitWidth, parent.width / 2)
        compact: true
        visible: root.showKeybinds
        capabilityTitle: qsTr("%1: %2").arg(modesCapability.title).arg(option.title)
        keybindSettings: root.keybindSettings
        deviceName: root.keybindDeviceName
        bindingTarget: Object.assign({}, root.keybindTarget, { action: "set_mode", value: option.modeId })
      }
    }

    function updateCurrentIndexFromValue() {
      for (let i = 0; i < count; ++i) {
        if (model.get(i).id === selectedModeId) {
          if (currentIndex !== i) {
            currentIndex = i;
          }

          return;
        }
      }
      currentIndex = -1;
    }
  }
}
