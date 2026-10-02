import QtQuick
import QtQuick.Layouts
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
    target: capabilitiesModel

    function onDataLoaded() {
      console.log("Device data loading - OK");
    }
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

    onBackClicked: {
      deviceController.ForgetDevice();
      router.goBack();
    }
  }

  property var okCount: 0
  property var failCount: 0

  Connections {
    target: capabilitiesModel

    function onInitialized() {
      console.log("capabilities loaded");
      okCount++;
    }
  }

  Connections {
    target: propertiesModel

    function onInitialized() {
      console.log("propeties loaded");
      okCount++;
    }
  }

  StackLayout {
    width: parent.width
    anchors.top: topHeader.bottom
    anchors.bottom: parent.bottom

    // currentIndex: (okCount !== 2 && failCount === 0) ? 0 :
    //   ((failCount !== 0 || !deviceDataModel.isOnline) ? 1 : (okCount === 2 && failCount === 0) ? 2 : 1)

    currentIndex: 2

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
        }
      }
    }

    Flickable {
      contentHeight: contentItem.childrenRect.height
      clip: true
      interactive: true

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
