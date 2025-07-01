import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Models as Models
import YandexHomeDesktop.Capabilities as Capabilities

Item {
  height: 48

  Rectangle {
    anchors.fill: parent
    color: themes.headerBackground
    radius: 16
  }

  UI.DefaultText {
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    text: name
  }

  UI.MyComboBox {
    id: comboBox

    // model: modesModel
    model: Models.ModesFilterModel {
      sourceModel: modesModel
      allowedModes: [
        {
          "value": "fan_only"
        },
        {
          "value": "heat"
        },
        {
          "value": "cool"
        },
        {
          "value": "dry"
        },
        {
          "value": "auto"
        }
      ]
    }

    anchors.right: parent.right
    anchors.rightMargin: 12
    anchors.verticalCenter: parent.verticalCenter

    onCurrentIndexChanged: {
      const item = comboBox.model.getItem(comboBox.currentIndex);

      console.log(item);
    }
  }
}