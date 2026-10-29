import QtQuick
import YandexHomeDesktop.ViewModels
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

UI.PageSurface {
  id: root
  popupQuitVisible: false
  readonly property bool modalDialogOpen: snapshotDialog.dialogOpen

  Components.CameraSnapshotDialog {
    id: snapshotDialog
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: implicitHeight
  }

  UI.ErrorDialog {
    id: actionErrorDialog
    objectName: "deviceErrorDialog"

    dialogTitle: qsTr("Ошибка")
    dialogMessage: "!"
  }

  Connections {
    target: deviceViewModel

    function onDeviceIdChanged() { snapshotDialog.clear(); }

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
    anchors.bottom: snapshotDialog.top

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
      showKeybinds: settings.showKeybinds
      keybindSettings: keybindSettingsModel
      deviceId: deviceViewModel.deviceId
      deviceName: deviceViewModel.deviceData.name
      streamSource: deviceViewModel.videoStreamUrl
      streamLoading: deviceViewModel.videoStreamLoading
      streamFailed: deviceViewModel.videoStreamFailed
      onStreamRequested: deviceViewModel.RequestVideoStream()
      onStreamStopRequested: deviceViewModel.StopVideoStream()
      onSnapshotRequested: function(videoSink) { snapshotDialog.capture(videoSink); }
      capabilitiesSourceModel: deviceViewModel.capabilities
      propertiesSourceModel: deviceViewModel.properties
      capabilitiesTitle: qsTr("Умения")
      propertiesTitle: qsTr("Свойства")
    }
  }
}
