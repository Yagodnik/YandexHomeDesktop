import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  Rectangle {
    anchors.fill: parent
    color: themes.background
  }

  UI.ErrorDialog {
    id: actionErrorDialog

    dialogTitle: "Ошибка"
    dialogMessage: "!"
  }

  Connections {
    target: deviceModel

    function onDataLoaded() {
      console.log("Device data loading - OK");
    }

    function onDataLoadingFailed() {
      console.log("Device data loading - FAILED");
    }

    function onErrorOccurred(errorCode) {
      const error = errorCodes.GetDeviceError(errorCode);

      if (error == null) {
        actionErrorDialog.dialogMessage = "Произошла ошибка!";
      } else {
        actionErrorDialog.dialogMessage = error.short_description + "\n\n" + error.full_description;
      }

      actionErrorDialog.openDialog();
    }
  }

  Item {
    id: topHeader

    width: parent.width
    height: 48

    Rectangle {
      anchors.fill: parent
      color: themes.headerBackground
    }

    Image {
      id: backButton
      source: "qrc:/images/back.svg"

      property real rotationAngle: 0

      antialiasing: true
      layer.enabled: true
      layer.smooth: true
      layer.samples: 8

      fillMode: Image.PreserveAspectFit

      anchors.verticalCenter: parent.verticalCenter
      anchors.left: parent.left
      anchors.leftMargin: 12

      MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor

        onClicked: {
          router.goBack();
        }
      }
    }

    UI.DefaultText {
      text: "НУ ВОТ ПРЯМ ДЛИННОЕ НАЗВАНИЕ"
      anchors.verticalCenter: parent.verticalCenter
      anchors.horizontalCenter: parent.horizontalCenter
      color: themes.controlText
    }
  }

  UI.HeadingText {
    id: capabilitiesText
    text: "Умения"

    anchors.top: topHeader.bottom
    anchors.topMargin: 4
    anchors.left: parent.left
    anchors.leftMargin: 16
  }

  ListView {
    id: capabilitiesList
    anchors.top: capabilitiesText.bottom
    anchors.topMargin: 4
    // anchors.bottom: parent.bottom
    // anchors.bottomMargin: 10

    height: 100

    anchors.left: parent.left
    anchors.leftMargin: 16
    anchors.right: parent.right
    anchors.rightMargin: 16

    clip: true

    spacing: 10

    model: deviceModel

    delegate: Loader {
      source: delegateSource
      width: parent.width
    }
  }

  UI.HeadingText {
    id: propertiesText
    text: "Свойства"

    anchors.top: capabilitiesList.bottom
    anchors.topMargin: 4
    anchors.left: parent.left
    anchors.leftMargin: 16
  }

  ListView {
    id: propertiesList
    anchors.top: propertiesText.bottom
    anchors.topMargin: 4
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10

    anchors.left: parent.left
    anchors.leftMargin: 16
    anchors.right: parent.right
    anchors.rightMargin: 16

    clip: true

    spacing: 10

    model: ListModel {
      ListElement {
        name: "Влажность"
        value: 15
        unit: "%"
      }

      ListElement {
        name: "Уровень заряда"
        value: 55
        unit: "%"
      }

      ListElement {
        name: "Уровень воды"
        value: 80
        unit: "%"
      }
    }

    delegate: Item {
      width: parent.width
      height: 58

      Rectangle {
        anchors.fill: parent
        color: themes.headerBackground
        radius: 16
      }

      Item {
        id: top

        anchors.top: parent.top
        anchors.topMargin: 8

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

          text: value + unit
        }
      }

      UI.MySlider {
        id: valueSlider
        anchors.top: top.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12

        height: 12

        from: 0
        to: 100
        enabled: false

        value: model.value
      }
    }
  }
}