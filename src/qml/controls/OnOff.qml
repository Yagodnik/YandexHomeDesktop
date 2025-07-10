import QtQuick
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Capabilities as Capabilities
import Qt5Compat.GraphicalEffects

Item {
  id: root
  height: 64

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

  RowLayout {
    anchors.fill: parent

    Item {
      Layout.fillWidth: true
      Layout.preferredWidth: 1
      height: parent.height
      // Layout.alignment: Qt.AlignVCenter

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
          const capability_action = onOffCapability.Create(true);
          capabilitiesModel.UseCapability(model.index, capability_action);
        }
      }

      Column {
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter

        Item {
          width: 32
          height: 32
          anchors.horizontalCenter: parent.horizontalCenter

          Image {
            id: baseImage
            anchors.fill: parent
            source: "qrc:/images/on.svg"
          }

          ColorOverlay {
            anchors.fill: baseImage
            source: baseImage
            color: onOffCapability.value ? themes.accent : themes.inactive
          }
        }

        UI.DefaultText {
          text: "Вкл"
          anchors.horizontalCenter: parent.horizontalCenter
        }
      }
    }

    Item {
      Layout.fillWidth: true
      Layout.preferredWidth: 1
      height: parent.height
      // Layout.alignment: Qt.AlignVCenter

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
          const capability_action = onOffCapability.Create(false);
          capabilitiesModel.UseCapability(model.index, capability_action);
        }
      }

      Column {
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter

        Item {
          width: 32
          height: 32
          anchors.horizontalCenter: parent.horizontalCenter

          Image {
            id: baseImage2
            anchors.fill: parent
            source: "qrc:/images/on.svg"
          }

          ColorOverlay {
            anchors.fill: baseImage2
            source: baseImage2
            color: onOffCapability.value ? themes.inactive : themes.accent
          }
        }

        UI.DefaultText {
          text: "Выкл"
          anchors.horizontalCenter: parent.horizontalCenter
        }
      }
    }
  }


  // UI.DefaultText {
  //   anchors.left: parent.left
  //   anchors.leftMargin: 12
  //   anchors.verticalCenter: parent.verticalCenter
  //
  //   text: onOffCapability.title
  // }
  //
  // Item {
  //   anchors.right: parent.right
  //   anchors.rightMargin: 12
  //   anchors.verticalCenter: parent.verticalCenter
  //   width: switchControl.width
  //   height: switchControl.height
  //
  //   UI.MySwitch {
  //     id: switchControl
  //     anchors.fill: parent
  //     checked: onOffCapability.value
  //     // enabled: !model.busy
  //
  //     onToggled: function(checked) {
  //       const capability_action = onOffCapability.Create(checked);
  //       capabilitiesModel.UseCapability(model.index, capability_action);
  //     }
  //   }
  // }
}
