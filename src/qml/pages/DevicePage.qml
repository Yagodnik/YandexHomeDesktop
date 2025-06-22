import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  Rectangle {
    anchors.fill: parent
    color: themes.GetBackground()
  }

  Item {
    id: topHeader

    width: parent.width
    height: 48

    Rectangle {
      anchors.fill: parent
      color: themes.GetHeaderBackground()
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
      color: themes.GetControlText()
    }
  }

  ListView {
    anchors.top: topHeader.bottom
    anchors.topMargin: 16
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
        name: "Вкл/выкл"
        delegateSource: "qrc:/controls/OnOff.qml"
      }

      ListElement {
        name: "Яркость"
        delegateSource: "qrc:/controls/Range.qml"
      }

      // ListElement {
      //   name: "Цвета"
      //   delegateSource: "qrc:/controls/ColorSetting.qml"
      // }
    }

    delegate: Loader {
      source: delegateSource
      width: parent.width
    }
  }
}