import QtQuick
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Models as Models
import YandexHomeDesktop.Capabilities as Capabilities
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels

Item {
  id: root
  objectName: "deviceColorControl"
  width: parent.width
  implicitHeight: contentColumn.implicitHeight
  property int currentTab: 0
  property bool showKeybinds: false
  property KeybindSettingsModel keybindSettings: null
  property var keybindTarget: ({})
  property string keybindDeviceName
  property var deviceModelData: model
  readonly property int optionColumns: Math.max(1, Math.floor((width + 8) / (88 + 8)))
  readonly property real optionWidth: (width - (optionColumns - 1) * 8) / optionColumns

  Component.onCompleted: {
    if (colorSettingCapability.state.instance === "scene"
        || (!colorSettingCapability.supportsColors && !colorSettingCapability.supportsTemperatures))
      currentTab = 1;
  }

  Capabilities.ColorSetting {
    id: colorSettingCapability
    state: model.attributeState
    parameters: model.attributeParameters
  }

  ColumnLayout {
    id: contentColumn
    spacing: 12
    width: parent.width

    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Repeater {
        model: [
          { label: qsTr("Цвета"), index: 0 },
          { label: qsTr("Режимы"), index: 1 }
        ]

        delegate: Item {
          property int index: modelData.index
          objectName: "colorTab-" + index
          width: 80
          height: 36

          Rectangle {
            anchors.fill: parent
            radius: 45
            color: root.currentTab === index ? themes.accent : themes.inactive
          }

          UI.DefaultText {
            anchors.centerIn: parent
            width: parent.width
            text: modelData.label
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            color: themes.controlText2
            horizontalAlignment: Text.AlignHCenter
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.currentTab = index
          }
        }
      }
    }

    Loader {
      Layout.fillWidth: true
      sourceComponent: root.currentTab === 0 ? colorsPage : modesPage
    }
  }

  Component {
    id: colorsPage

    Grid {
      objectName: "colorOptions"
      width: root.width
      columns: root.optionColumns
      spacing: 8

      Repeater {
        model: Models.ColorsFilterModel {
          min: colorSettingCapability.temperatureMin
          max: colorSettingCapability.temperatureMax
          supportsColors: colorSettingCapability.supportsColors
          supportsTemperatures: colorSettingCapability.supportsTemperatures
          sourceModel: colorModel
        }

        delegate: Components.CapabilityOptionTile {
          objectName: model.isTemperature ? "temperatureOption-" + model.temperature : "colorOption-" + model.colorIndex
          width: root.optionWidth
          title: model.name
          showSwatch: true
          swatchColor: model.color
          isSelected: model.isTemperature
            ? colorSettingCapability.state.instance === "temperature_k"
              && colorSettingCapability.value === model.temperature
            : (colorSettingCapability.state.instance === "rgb"
               || colorSettingCapability.state.instance === "hsv")
              && colorSettingCapability.MatchesColor(model.color)
          showKeybinds: root.showKeybinds
          keybindSettings: root.keybindSettings
          deviceName: root.keybindDeviceName
          bindingTarget: Object.assign({}, root.keybindTarget, {
            action: model.isTemperature ? "set_temperature" : "set_color",
            value: model.isTemperature ? String(model.temperature) : model.color.toString().toLowerCase()
          })
          onSelected: {
            const action = model.isTemperature
              ? colorSettingCapability.Create(model.temperature)
              : colorSettingCapability.Create(model.color);
            capabilitiesModel.UseCapability(root.deviceModelData.index, action);
          }
        }
      }
    }
  }

  Component {
    id: modesPage

    Grid {
      objectName: "sceneOptions"
      width: root.width
      columns: root.optionColumns
      spacing: 8

      Repeater {
        model: Models.ColorModesFilterModel {
          allowedScenes: colorSettingCapability.availableScenes
          sourceModel: colorModesModel
        }

        delegate: Components.CapabilityOptionTile {
          objectName: "sceneOption-" + model.modeId
          width: root.optionWidth
          title: model.name
          imageSource: model.image
          isSelected: colorSettingCapability.state.instance === "scene"
            && colorSettingCapability.value === model.modeId
          showKeybinds: root.showKeybinds
          keybindSettings: root.keybindSettings
          deviceName: root.keybindDeviceName
          bindingTarget: Object.assign({}, root.keybindTarget, { action: "set_scene", value: model.modeId })
          onSelected: {
            const action = colorSettingCapability.Create(model.modeId);
            capabilitiesModel.UseCapability(root.deviceModelData.index, action);
          }
        }
      }
    }
  }
}
