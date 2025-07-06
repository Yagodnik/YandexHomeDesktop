import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Capabilities as Capabilities

Item {
  id: root
  height: 48

  Capabilities.OnOff {
    id: onOffCapability

    state: model.attributeState
    parameters: model.attributeParameters
    titlesList: iotTitles
  }

  Rectangle {
    anchors.fill: parent
    color: themes.headerBackground
    radius: 16
  }

  UI.DefaultText {
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    text: onOffCapability.title
  }

  Item {
    anchors.right: parent.right
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter
    width: switchControl.width
    height: switchControl.height

    UI.MySwitch {
      id: switchControl
      anchors.fill: parent
      checked: onOffCapability.value
      // enabled: !model.busy

      onToggled: function(checked) {
        const capability_action = onOffCapability.Create(checked);
        capabilitiesModel.UseCapability(model.index, capability_action);
      }
    }
  }
}
