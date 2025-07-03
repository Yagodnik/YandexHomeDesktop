import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Models as Models
import YandexHomeDesktop.Capabilities as Capabilities

Item {
  height: 48

  property var deviceModelData: model

  Capabilities.Modes {
    id: modesCapability

    state: model.attributeState
    parameters: model.attributeParameters
    titlesList: capabilitiesTitles
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

    text: modesCapability.title
  }

  UI.MyComboBox {
    id: comboBox
    width: 120

    textRole: "displayText"

    model: Models.ModesFilterModel {
      sourceModel: modesModel

      // allowedModes: [
      //   { "value": "fan_only" },
      //   { "value": "heat"     },
      //   { "value": "cool"     },
      //   { "value": "dry"      },
      //   { "value": "auto"     }
      // ]

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

    onCurrentIndexChanged: {
      console.log(deviceModelData.index);

      const idx = currentIndex;
      const id = model.get(idx).id;
      if (selectedModeId !== id) {
        selectedModeId = id;
        console.log("New id:", id);
      }

      const capability_action = modesCapability.Create(selectedModeId);
      capabilitiesModel.UseCapability(model.index, capability_action);
    }

    function updateCurrentIndexFromValue() {
      for (let i = 0; i < model.count; ++i) {
        if (model.get(i).id === selectedModeId) {
          if (currentIndex !== i) {
            currentIndex = i;
          }

          break;
        }
      }
    }
  }
}