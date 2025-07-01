import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Models as Models
import YandexHomeDesktop.Capabilities as Capabilities

Item {
  id: root
  width: parent.width
  implicitHeight: contentColumn.implicitHeight
  property int currentTab: 0

  property var deviceModelData: model

  Capabilities.ColorSetting {
    id: colorSettingCapability

    state: model.attributeState
    parameters: model.attributeParameters
  }

  ColumnLayout {
    id: contentColumn
    spacing: 12
    width: parent.width

    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Repeater {
        model: [
          { label: "Цвета", index: 0 },
          { label: "Режимы", index: 1 }
        ]

        delegate: Item {
          property int index: modelData.index
          width: 80; height: 36

          Rectangle {
            anchors.fill: parent
            radius: 45
            color: root.currentTab === index ? themes.accent : themes.inactive
          }

          UI.DefaultText {
            anchors.centerIn: parent
            width: parent.width
            text: modelData.label
            font.pointSize: 14
            wrapMode: Text.WordWrap
            color: themes.controlText2
            horizontalAlignment: Text.AlignHCenter
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.currentTab = index
          }
        }
      }
    }

    Loader {
      id: tabLoader
      Layout.fillWidth: true
      sourceComponent: root.currentTab === 0 ? colorsPage : modesPage
    }
  }

  Component {
    id: colorsPage

    Flow {
      width: root.width
      spacing: 8
      height: childrenRect.height

      Repeater {
        // model: 20
        // model: colorModel

        model: Models.ColorsFilterModel {
          // min: colorSettingCapability.GetTemperatureMin()
          // max: colorSettingCapability.GetTemperatureMax()

          min: colorSettingCapability.temperatureMin
          max: colorSettingCapability.temperatureMax

          sourceModel: colorModel

          Component.onCompleted: {
            console.log("Colors Min/Max: ", min, max);
          }
        }

        delegate: Rectangle {
          width: Math.max(72, root.width / 5 - 10)
          height: 90
          radius: 12
          color: themes.headerBackground

          Column {
            anchors.centerIn: parent
            spacing: 6

            Rectangle {
              width: 32
              height: 32
              radius: 16
              // color: Qt.hsla(index / 20.0, 1.0, 0.5, 1.0)
              color: model.color

              anchors.horizontalCenter: parent.horizontalCenter
            }

            UI.DefaultText {
              width: parent.width
              text: model.name
              font.pointSize: 10
              color: themes.controlText
              wrapMode: Text.WordWrap
              anchors.horizontalCenter: parent.horizontalCenter
              horizontalAlignment: Text.AlignHCenter
            }
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              console.log("Selected color:", index, model.color);
              console.log("Model index:", deviceModelData.index);

              let capability_action = null;

              if (isTemperature) {
                capability_action = colorSettingCapability.Create(model.temperature);
              } else {
                capability_action = colorSettingCapability.Create(model.color);
              }

              capabilitiesModel.UseCapability(deviceModelData.index, capability_action);
            }
          }
        }
      }
    }
  }

  Component {
    id: modesPage

    Flow {
      width: root.width
      spacing: 8
      height: childrenRect.height

      Repeater {
        // model: 12
        // model: modesModel

        model: Models.ColorModesFilterModel {
          // allowedScenes: colorSettingCapability.GetAvailableScenes()
          allowedScenes: colorSettingCapability.availableScenes
          sourceModel: colorModesModel
        }

        delegate: Rectangle {
          width: Math.max(90, root.width / 4 - 10)
          height: 90
          radius: 12
          color: themes.headerBackground

          Column {
            anchors.centerIn: parent
            spacing: 6

            // Rectangle {
            //   width: 32
            //   height: 32
            //   radius: 16
            //   color: "red"
            //
            //   anchors.horizontalCenter: parent.horizontalCenter
            // }

            Image {
              id: icon
              width: 32
              height: 32
              anchors.horizontalCenter: parent.horizontalCenter

              source: image

              onStatusChanged: {
                if (status === Image.Error) {
                  console.warn("Image failed to load:", source);
                  source = "qrc:/images/modes/unknown.svg";
                }
              }
            }


            UI.DefaultText {
              text: name
              font.pointSize: 10
              color: themes.controlText

              anchors.horizontalCenter: parent.horizontalCenter
            }
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              console.log("Selected mode:", index, modeId);
              console.log("Model index:", deviceModelData.index);
              console.log("Selected mode:", modeId);

              const capability_action = colorSettingCapability.Create(modeId);
              capabilitiesModel.UseCapability(deviceModelData.index, capability_action);
            }
          }
        }
      }
    }
  }
}
