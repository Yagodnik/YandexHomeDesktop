import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  height: 48

  Connections {
    target: deviceModel

    function onDataUpdated() {
      const value = deviceModel.GetValue(model.index);

      switchControl.checked = value;
    }
  }

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

  UI.MySwitch {
    id: switchControl
    anchors.right: parent.right
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    onToggled: function(checked) {
      const capability_info = deviceModel.GetCapabilityInfo(model.index);
      const capability_action = capabilityFactory.CreateOnOff(capability_info, checked);

      deviceModel.Test(capability_info);
      deviceModel.UseCapability(model.index, capability_action);
    }
  }
}