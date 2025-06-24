import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  width: parent.width
  implicitHeight: contentColumn.implicitHeight
  property int currentTab: 0

  property var deviceModelData: model

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
            color: root.currentTab === index ? themes.GetAccent() : themes.GetInactive()
          }

          UI.DefaultText {
            anchors.centerIn: parent
            text: modelData.label
            font.pointSize: 14
            color: themes.GetControlText()
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
        model: 20

        delegate: Rectangle {
          width: Math.max(72, root.width / 5 - 10)
          height: 90
          radius: 12
          color: themes.GetHeaderBackground()

          Column {
            anchors.centerIn: parent
            spacing: 6

            Rectangle {
              width: 32
              height: 32
              radius: 16
              color: Qt.hsla(index / 20.0, 1.0, 0.5, 1.0)

              anchors.horizontalCenter: parent.horizontalCenter
            }

            UI.DefaultText {
              text: "Цвет " + index
              font.pointSize: 10
              color: themes.GetControlText()
              anchors.horizontalCenter: parent.horizontalCenter
            }
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              const color = Qt.hsla(index / 20.0, 1.0, 0.5, 1.0);
              console.log("Selected color:", index, color);
              console.log("Model index:", deviceModelData.index);

              const capability_info = deviceModel.GetCapabilityInfo(deviceModelData.index);
              const capability_action = capabilityFactory.CreateColorSetting(capability_info, color);

              deviceModel.UseCapability(deviceModelData.index, capability_action);
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
        model: 12

        delegate: Rectangle {
          width: Math.max(90, root.width / 4 - 10)
          height: 90
          radius: 12
          color: themes.GetHeaderBackground()

          Column {
            anchors.centerIn: parent
            spacing: 6

            Rectangle {
              width: 32
              height: 32
              radius: 16
              color: "red"

              anchors.horizontalCenter: parent.horizontalCenter
            }

            UI.DefaultText {
              text: "Режим " + index
              font.pointSize: 10
              color: themes.GetControlText()

              anchors.horizontalCenter: parent.horizontalCenter
            }
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              console.log("Selected mode:", index, color);
              console.log("Model index:", deviceModelData.index);

              const modes = [
                "northern",
                "christmas",
                "fairy",
                "snake",
                "alice",
                "party",
                "jungle",
                "neon",
                "night",
                "ocean",
                "romance"
              ];
              const mode_name = modes[index];

              console.log("Selected mode:", mode_name);

              const capability_info = deviceModel.GetCapabilityInfo(deviceModelData.index);
              const capability_action = capabilityFactory.CreateColorSetting(capability_info, mode_name);

              deviceModel.UseCapability(deviceModelData.index, capability_action);
            }
          }
        }
      }
    }
  }
}
