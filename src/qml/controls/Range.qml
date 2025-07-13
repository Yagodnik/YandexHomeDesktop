import QtQuick
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Capabilities as Capabilities

Item {
  id: rangeControl

  // height: 74
  // height: 90

  Capabilities.Range {
    id: rangeCapability

    state: model.attributeState
    parameters: model.attributeParameters
    titlesList: iotTitles
    units: unitsList
  }

  property var randomAccess: rangeCapability.supportsRandomAccess

  // 90 - for random access, 74 - for slider
  property var properHeight: randomAccess ? 74 : 90

  height: properHeight

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

      text: rangeCapability.title
    }

    UI.DefaultText {
      id: valueText

      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter

      text: rangeSlider.value + rangeCapability.unit
      // text: rangeCapability.formattedValue
    }
  }

  property var brightness: 50

  // Choosing on of the controls according to random access support
  // Note: https://yandex.ru/dev/dialogs/smart-home/doc/ru/concepts/range#discovery

  Item {
    id: relativeRangeControl

    visible: !randomAccess

    anchors.top: top.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.right: parent.right
    anchors.rightMargin: 12

    height: 32

    RowLayout {
      anchors.fill: parent

      Item {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        height: parent.height

        Rectangle {
          anchors.fill: parent
          radius: 16
          color: themes.accent
        }

        UI.DefaultText {
          anchors.centerIn: parent
          text: "+"
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor

          onClicked: {
            const capability_action = rangeCapability.Create(rangeCapability.precision);
            capabilitiesModel.UseCapability(model.index, capability_action);
          }
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        height: parent.height

        Rectangle {
          anchors.fill: parent
          radius: 16
          color: themes.accent
        }

        UI.DefaultText {
          anchors.centerIn: parent
          text: "-"
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor

          onClicked: {
            const capability_action = rangeCapability.Create(-rangeCapability.precision);
            capabilitiesModel.UseCapability(model.index, capability_action);
          }
        }
      }
    }
  }

  UI.MySlider {
    id: rangeSlider
    anchors.top: top.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 12
    anchors.right: parent.right
    anchors.rightMargin: 12

    visible: randomAccess

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

