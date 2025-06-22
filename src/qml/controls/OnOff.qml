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
      console.log("On/Off", index, root.index, model.index);

      /*
      * var capability = deviceModel.GetOnOffCapability(model.index);
      * capability.SetValue(true);
      * */

      /*
      * var capability = deviceModel.GetColorSettingCapability(model.index);
      * capability.SetColor(true);
      * */

      /*
      * deviceModel.UseCapability(model.index, true);
      * deviceModel.UseCapability(model.index, 50);
      * deviceModel.UseCapability(model.index, "#ababeb");
      *
      * */
    }
  }
}