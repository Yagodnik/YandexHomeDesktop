import QtQuick 2.15
import QtQuick.Layouts 2.15
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  QtObject {
    id: shaderParams
    property real t: 0

    SequentialAnimation on t {
      loops: Animation.Infinite
      NumberAnimation {
        to: 1
        duration: 600
        easing.type: Easing.InBack
      }
      NumberAnimation {
        to: 0
        duration: 600
        easing.type: Easing.InBack
      }
    }
  }

  Flickable {
    id: flickable
    anchors.fill: parent
    contentWidth: parent.width
    contentHeight: contentItem.implicitHeight

    interactive: true
    flickableDirection: Flickable.VerticalFlick

    Item {
      id: contentItem
      width: flickable.width

      UI.HeadingText {
        id: heading
        text: "Настройки"
        width: parent.width
      }

      Column {
        width: parent.width
        anchors.top: heading.bottom
        anchors.topMargin: 12
        spacing: 4

        Components.AccountDetails {}

        Item {
          width: parent.width
          height: details2.height + 20

          Rectangle {
            anchors.fill: parent
            color: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 1)
            radius: 16
          }

          Column {
            id: details2
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            UI.DefaultText {
              text: "Tray-режим"
            }

            UI.SubheadingText {
              text: "Приложение будет отображаться\nкак иконка на панели задач"
            }
          }

          UI.MySwitch {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            onToggled: function(checked) {
              if (checked) {
                platformService.ShowOnlyInTray();
              } else {
                platformService.ShowAsApp();
              }
            }
          }
        }
      }
    }
  }
}
