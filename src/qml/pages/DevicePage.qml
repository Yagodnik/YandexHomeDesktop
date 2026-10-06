import QtQuick
import YandexHomeDesktop.ViewModels
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  id: root

  UI.ErrorDialog {
    id: actionErrorDialog
    objectName: "deviceErrorDialog"

    dialogTitle: qsTr("Ошибка")
    dialogMessage: "!"
  }

  Connections {
    target: deviceViewModel

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
    title: deviceViewModel.deviceData.name
    online: deviceViewModel.deviceData.isOnline ?? false

    onBackClicked: {
      deviceViewModel.ForgetDevice();
      router.goBack();
    }
  }

  UI.PageStates {
    id: deviceStates
    objectName: "deviceStates"
    width: parent.width
    anchors.top: topHeader.bottom
    anchors.bottom: parent.bottom

    currentIndex: deviceViewModel.state

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
        deviceViewModel.TryReloadDevice();
      }
    }

    Components.DeviceControlsPane {
      capabilitiesSourceModel: deviceViewModel.capabilities
      propertiesSourceModel: deviceViewModel.properties
      capabilitiesTitle: qsTr("Умения")
      propertiesTitle: qsTr("Свойства")
    }
  }
}
