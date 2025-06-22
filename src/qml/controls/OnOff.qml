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

  UI.MySwitch {
    anchors.right: parent.right
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    onToggled: function(checked) {
      const capability_info = deviceModel.GetCapabilityInfo(model.index);

      deviceModel.Test(capability_info);

      /*
      * var capability_info = deviceModel.GetCapabilityInfo(model.index);
      * deviceModel.UseCapability(
      *   model.index,
      *   actionFactory.CreateOnOffAction(
      *     capability_info, true
      *   )
      * );
      * */
    }
  }
}