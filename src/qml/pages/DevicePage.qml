import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root

  Rectangle {
    anchors.fill: parent
    color: themes.background
  }

  UI.ErrorDialog {
    id: actionErrorDialog

    dialogTitle: qsTr("Ошибка")
    dialogMessage: "!"
  }

  Connections {
    target: deviceController

    function onErrorOccurred(errorCode) {
      const error = errorCodes.GetDeviceError(errorCode);

      if (error == null) {
        actionErrorDialog.dialogMessage = qsTr("Произошла ошибка!");
      } else {
        actionErrorDialog.dialogMessage = error.short_description + "\n\n" + error.full_description;
      }

      actionErrorDialog.openDialog();
    }
  }

  Components.DeviceHeader {
    id: topHeader
    width: parent.width
    title: deviceDataModel.name
    online: deviceDataModel.isOnline ?? false

    onBackClicked: {
      deviceController.ForgetDevice();
      router.goBack();
    }
  }

  property var okCount: 0

  function initializationOk() {
    okCount++;
    if (okCount >= 3) {
      deviceStates.currentIndex = 2;
    }
  }

  function initializationFailed() {
    deviceStates.currentIndex = 1;
  }

  Connections {
    target: capabilitiesModel

    function onInitialized() {
      initializationOk();
    }

    function onInitializeFailed() {
      initializationFailed();
    }
  }

  Connections {
    target: propertiesModel

    function onInitialized() {
      initializationOk();
    }

    function onInitializeFailed() {
      initializationFailed();
    }
  }

  Connections {
    target: deviceDataModel

    function onInitialized() {
      initializationOk();
    }

    function onInitializeFailed() {
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
          text: qsTr("Нет связи с устройством")
          anchors.horizontalCenter: parent.horizontalCenter
        }

        UI.MyButton {
          text: qsTr("Попробовать снова")
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
        anchors.right: parent.right
        anchors.rightMargin: 3

        contentItem: Rectangle {
          implicitWidth: 10
          radius: 16
          color: themes.headerBackground
        }
      }

      Column {
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: 4
        spacing: 4

        Components.DeviceAttributeSection {
          width: parent.width
          title: qsTr("Умения")
          sourceModel: capabilitiesModel
        }

        Components.DeviceAttributeSection {
          width: parent.width
          title: qsTr("Свойства")
          sourceModel: propertiesModel
          bottomSpace: 12
        }
      }
    }
  }
}
