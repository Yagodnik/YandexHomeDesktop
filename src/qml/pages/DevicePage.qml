import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  id: root

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

  property int okCount: 0

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

  UI.PageStates {
    id: deviceStates
    objectName: "deviceStates"
    width: parent.width
    anchors.top: topHeader.bottom
    anchors.bottom: parent.bottom

    currentIndex: 0

    UI.LoadingPane {
      active: deviceStates.currentIndex === 0
      indicatorSize: 32
      strokeWidth: 3
    }

    UI.RetryPane {
      iconSource: "qrc:/images/warning.svg"
      message: qsTr("Нет связи с устройством")
      buttonText: qsTr("Попробовать снова")
      onRetryRequested: {
        root.okCount = 0;
        deviceStates.currentIndex = 0;
        deviceController.TryReloadDevice();
      }
    }

    Components.DeviceControlsPane {
      capabilitiesSourceModel: capabilitiesModel
      propertiesSourceModel: propertiesModel
      capabilitiesTitle: qsTr("Умения")
      propertiesTitle: qsTr("Свойства")
    }
  }
}
