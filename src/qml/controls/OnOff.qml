import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 48

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
    radius: 16
  }

  UI.DefaultText {
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    text: name
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
      checked: model.deviceState["value"]
      // enabled: !model.busy

      onToggled: function(checked) {
        const capability_info = deviceModel.GetCapabilityInfo(model.index);
        const capability_action = capabilityFactory.CreateOnOff(capability_info, checked);

        deviceModel.UseCapability(model.index, capability_action);
      }
    }

    // UI.MyProgressIndicator {
    //   anchors.centerIn: parent
    //   visible: model.busy
    //
    //   width: 32
    //   height: 32
    // }
  }
}
