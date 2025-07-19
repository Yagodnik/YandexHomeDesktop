import QtQuick
import QtQuick.Layouts
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
      text: deviceDataModel.name
      anchors.verticalCenter: parent.verticalCenter
      anchors.horizontalCenter: parent.horizontalCenter
      color: themes.controlText
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
          text: "Нет связи с устройством"
          anchors.horizontalCenter: parent.horizontalCenter
        }

        UI.MyButton {
          text: "Попробовать снова"
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