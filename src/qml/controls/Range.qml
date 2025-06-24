import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: rangeControl
  height: 74

  Connections {
    target: deviceModel

    function onDataUpdated(index) {
      if (index !== model.index) {
        return;
      }
      // if (busy) {
      //   return;
      // }

      // const state = deviceModel.GetState(model.index);
      //
      // console.log("Assigning new value = ", state["value"]);
      //
      // rangeSlider.value = state["value"];
    }
  }

  Rectangle {
    anchors.fill: parent
    color: themes.GetHeaderBackground()
    radius: 16
  }

  Item {
    id: top

    anchors.top: rangeControl.top
    anchors.topMargin: 16

    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.right: parent.right
    anchors.rightMargin: 12

    height: 20

    UI.DefaultText {
      id: propertyTitle

      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter

      text: name
    }

    UI.DefaultText {
      id: valueText

      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter

      text: rangeSlider.value + "%"
    }
  }

  property var brightness: 50

  UI.MySlider {
    id: rangeSlider
    anchors.top: top.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.right: parent.right
    anchors.rightMargin: 12

    height: 12

    from: 0
    to: 100
    stepSize: 1

    value: deviceState["value"]

    onPressedChanged: {
      if (!pressed) {
        console.log("Applying slider updates")

        const capability_info = deviceModel.GetCapabilityInfo(model.index);
        const capability_action = capabilityFactory.CreateRange(capability_info, value);

        deviceModel.UseCapability(model.index, capability_action);
      }
    }
  }
}

