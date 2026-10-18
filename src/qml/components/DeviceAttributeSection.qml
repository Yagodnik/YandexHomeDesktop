import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.ViewModels

Column {
  id: root
  spacing: 4

  property string title
  property var sourceModel: null
  property bool showKeybinds: false
  property KeybindSettingsModel keybindSettings: null
  property string deviceId
  property string deviceName

  visible: sourceModel !== null && sourceModel.count !== 0

  UI.HeadingText {
    text: root.title
    x: 16
  }

  Column {
    id: attributes
    width: root.width - 32
    x: 16
    spacing: 10

    Repeater {
      model: root.sourceModel

      delegate: Item {
        id: attribute
        readonly property var baseTarget: ({
          device_id: root.deviceId,
          capability_type: model.name ?? "",
          instance: model.attributeParameters?.instance ?? (model.name === "devices.capabilities.on_off" ? "on" : "")
        })
        width: parent.width
        height: attributeLoader.height + (keybind.visible ? 22 : 0)

        Loader {
          id: attributeLoader
          source: delegateSource
          width: parent.width
        }

        Binding {
          target: attributeLoader.item
          property: "showKeybinds"
          value: root.showKeybinds
          when: attributeLoader.item !== null && attributeLoader.item.showKeybinds !== undefined
        }

        Binding {
          target: attributeLoader.item
          property: "keybindSettings"
          value: root.keybindSettings
          when: attributeLoader.item !== null && attributeLoader.item.keybindSettings !== undefined
        }

        Binding {
          target: attributeLoader.item
          property: "keybindTarget"
          value: attribute.baseTarget
          when: attributeLoader.item !== null && attributeLoader.item.keybindTarget !== undefined
        }

        Binding {
          target: attributeLoader.item
          property: "keybindDeviceName"
          value: root.deviceName
          when: attributeLoader.item !== null && attributeLoader.item.keybindDeviceName !== undefined
        }

        Components.CapabilityKeybind {
          id: keybind
          objectName: "capabilityKeybind"
          anchors.right: parent.right
          anchors.rightMargin: 8
          y: attributeLoader.height - 4
          visible: root.showKeybinds && capabilityTitle.length > 0
          capabilityTitle: attributeLoader.item?.keybindTitle ?? ""
          keybindSettings: root.keybindSettings
          deviceName: root.deviceName
          bindingTarget: Object.assign({}, attribute.baseTarget, { action: "toggle", value: "" })
        }
      }
    }
  }
}
