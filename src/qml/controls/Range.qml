import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Capabilities as Capabilities

Item {
  id: rangeControl
  height: 74

  Capabilities.Range {
    id: rangeCapability

    state: model.attributeState
    parameters: model.attributeParameters
  }

  Rectangle {
    anchors.fill: parent
    color: themes.headerBackground
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

    from: rangeCapability.min
    to: rangeCapability.max
    stepSize: rangeCapability.precision

    value: rangeCapability.value

    onPressedChanged: {
      if (!pressed) {
        const capability_action = rangeCapability.Create(value);
        capabilitiesModel.UseCapability(model.index, capability_action);
      }
    }
  }
}

