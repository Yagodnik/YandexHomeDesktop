import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components
import YandexHomeDesktop.Models as Models

Item {
  id: root

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
    target: deviceController

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
    z: 200

    Rectangle {
      id: headerBackground
      anchors.fill: parent
      color: themes.headerBackground
    }

    DropShadow {
      anchors.fill: headerBackground
      source: headerBackground
      radius: 12
      samples: 16
      horizontalOffset: 0
      verticalOffset: 2
      // color: "#66000000" // ?
      // color: Qt.rgba(0, 32 / 255, 128 / 255, 0.04)
      color: themes.shadowColor
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
          deviceController.ForgetDevice();
          router.goBack();
        }
      }
    }

    UI.DefaultText {
      id: deviceTitleText
      text: deviceDataModel.name
      anchors.verticalCenter: parent.verticalCenter
      anchors.horizontalCenter: parent.horizontalCenter
      color: themes.controlText
    }

    MouseArea {
      id: hoverArea
      anchors.fill: deviceOffile
      hoverEnabled: !root.deviceOnline
    }

    Image {
      id: deviceOffile

      anchors.left: deviceTitleText.right
      anchors.leftMargin: 4
      anchors.verticalCenter: deviceTitleText.verticalCenter
      source: "qrc:/images/warning.svg"
      visible: !root.deviceOnline

      ToolTip.visible: hoverArea.containsMouse
      ToolTip.delay: 300
      ToolTip.text: "Устройство оффлайн!"
    }
  }

  property var deviceOnline: deviceDataModel.isOnline ?? false
  property var okCount: 0
  property var failCount: 0

  function initializationOk() {
    okCount++;

    if (okCount >= 3) {
      deviceStates.currentIndex = 2;
    }
  }

  function initializationFailed() {
    // Switching to error screen
    deviceStates.currentIndex = 1;
  }

  Connections {
    target: capabilitiesModel

    function onDataLoaded() {
      console.log("Capabilities Model: Device data loading - OK");
    }

    function onInitialized() {
      console.log("Capabilities Model: Capabilities loaded");
      initializationOk();
    }

    function onInitializeFailed() {
      console.log("Capabilities Model: Capabilities failed");
      initializationFailed();
    }
  }

  Connections {
    target: propertiesModel

    function onInitialized() {
      console.log("PropertiesModel: Properties loaded");
      initializationOk();
    }

    function onInitializeFailed() {
      console.log("PropertiesModel: Properties failed");
      initializationFailed();
    }
  }

  Connections {
    target: deviceDataModel

    function onInitialized() {
      console.log("DeviceDataModel: Data loaded");
      initializationOk();
    }

    function onInitializeFailed() {
      console.log("DeviceDataModel: Data loading failed");
      initializationFailed();
    }
  }

  StackLayout {
    id: deviceStates
    width: parent.width
    anchors.top: topHeader.bottom
    anchors.bottom: parent.bottom

    currentIndex: 0

    Item {
      UI.MyProgressIndicator {
        width: 32
        height: 32

        anchors.centerIn: parent
      }
    }

    Item {
      Column {
        spacing: 4
        anchors.centerIn: parent

        Image {
          width: 32
          height: 32
          source: "qrc:/images/warning.svg"
          anchors.horizontalCenter: parent.horizontalCenter
        }

        UI.DefaultText {
          text: "Что-то пошло не так"
          anchors.horizontalCenter: parent.horizontalCenter
        }

        UI.MyButton {
          text: "Попробовать снова"
          anchors.horizontalCenter: parent.horizontalCenter

          onClicked: {
            root.okCount = 0;
            deviceStates.currentIndex = 0;
            deviceController.TryReloadDevice();
          }
        }
      }
    }

    Flickable {
      id: deviceControlsList
      contentHeight: contentItem.childrenRect.height
      clip: true
      interactive: true

      ScrollBar.vertical: ScrollBar {
        width: 10
        policy: deviceControlsList.contentHeight > deviceControlsList.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

        opacity: hovered ? 1.0 : 0.2
        Behavior on opacity {
          NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
          }
        }

        anchors.right: parent.right
        anchors.rightMargin: 3

        contentItem: Rectangle {
          implicitWidth: 10
          radius: 16
          color: themes.headerBackground
        }

        background: Item {}
      }

      Column {
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: 4
        spacing: 4

        UI.HeadingText {
          id: capabilitiesText
          text: "Умения"

          visible: capabilitiesModel.count !== 0

          // anchors.top: parent.top
          // anchors.topMargin: 4
          anchors.left: parent.left
          anchors.leftMargin: 16
        }

        ListView {
          id: capabilitiesList
          // anchors.top: capabilitiesText.bottom
          // anchors.topMargin: 4
          height: contentHeight

          visible: capabilitiesModel.count !== 0

          anchors.left: parent.left
          anchors.leftMargin: 16
          anchors.right: parent.right
          anchors.rightMargin: 16

          clip: true
          interactive: false

          spacing: 10

          model: capabilitiesModel

          delegate: Loader {
            source: delegateSource
            width: parent.width
          }
        }

        UI.HeadingText {
          id: propertiesText
          text: "Свойства"

          visible: propertiesModel.count !== 0

          // anchors.top: capabilitiesList.bottom
          // anchors.topMargin: 4
          anchors.left: parent.left
          anchors.leftMargin: 16
        }

        ListView {
          id: propertiesList
          // anchors.top: propertiesText.bottom
          // anchors.topMargin: 4

          height: contentHeight + 12

          anchors.left: parent.left
          anchors.leftMargin: 16
          anchors.right: parent.right
          anchors.rightMargin: 16

          clip: true
          interactive: false

          spacing: 10

          model: propertiesModel

          delegate: Loader {
            source: delegateSource
            width: parent.width
          }
        }
      }
    }
  }
}