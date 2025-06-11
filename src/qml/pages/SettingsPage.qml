import QtQuick 2.15
import QtQuick.Layouts 2.15
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

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

      Rectangle {
        id: background
        anchors.fill: userDetails
        color: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 1)
        radius: 16
      }

      DropShadow {
        anchors.fill: background
        source: background
        horizontalOffset: 0
        verticalOffset: 2
        radius: 6
        samples: 16
        color: Qt.rgba(0, 32 / 255, 128 / 255, 0.04)
      }

      Item {
        id: userDetails
        width: parent.width
        anchors.top: heading.bottom
        anchors.topMargin: 12
        height: 56

        property bool loading: true

        Rectangle {
          id: avatar
          width: 32
          height: 32
          radius: 45
          color: userDetails.loading ? "#e0e0e0" : "red"

          anchors.left: parent.left
          anchors.leftMargin: 5
          anchors.verticalCenter: parent.verticalCenter

          layer.enabled: userDetails.loading
          layer.smooth: true
          layer.effect: ShaderEffect {
            property real t: shaderParams.t
            fragmentShader: "qrc:/shaders/highlight.frag.qsb"
          }
        }

        Column {
          id: details
          anchors.left: avatar.right
          anchors.leftMargin: 6
          anchors.verticalCenter: parent.verticalCenter
          spacing: 4

          Rectangle {
            width: userDetails.loading ? 120 : 0
            height: userDetails.loading ? 14 : 0
            visible: userDetails.loading
            color: "#e0e0e0"
            radius: 16

            layer.enabled: userDetails.loading
            layer.smooth: true
            layer.effect: ShaderEffect {
              property real t: shaderParams.t
              fragmentShader: "qrc:/shaders/highlight.frag.qsb"
            }
          }

          Rectangle {
            width: userDetails.loading ? 180 : 0
            height: userDetails.loading ? 12 : 0
            visible: userDetails.loading
            color: "#e0e0e0"
            radius: 16

            layer.enabled: userDetails.loading
            layer.smooth: true
            layer.effect: ShaderEffect {
              property real t: shaderParams.t
              fragmentShader: "qrc:/shaders/highlight.frag.qsb"
            }
          }

          UI.DefaultText {
            visible: !userDetails.loading
            text: "Artem Yagodnik"
          }

          UI.SubheadingText {
            visible: !userDetails.loading
            text: "YagodnikArtem2@yandex.ru"
          }
        }

        Image {
          id: logoutButton
          source: "qrc:/images/logout.svg"
          // visible: !userDetails.loading

          antialiasing: true
          layer.enabled: true
          layer.smooth: true
          layer.samples: 8

          fillMode: Image.PreserveAspectFit
          scale: 0.6
          anchors.right: parent.right
          anchors.rightMargin: 5
          anchors.verticalCenter: parent.verticalCenter

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              authorizationService.Logout();

              console.log("Logout...")
            }
          }
        }
      }

      // Column {
      //   id: items
      //
      //   width: parent.width
      //   anchors.top: heading.bottom
      //   anchors.topMargin: 5
      //
      //   Item {
      //     width: parent.width
      //     height: details.height + 20
      //
      //     Column {
      //       id: details
      //       anchors.left: parent.left
      //       anchors.leftMargin: 10
      //       anchors.verticalCenter: parent.verticalCenter
      //
      //       UI.DefaultText {
      //         text: "Tray-режим"
      //       }
      //
      //       UI.SubheadingText {
      //         text: "Приложение будет отображаться\nкак иконка на панели задач"
      //       }
      //     }
      //
      //     UI.MySwitch {
      //       anchors.right: parent.right
      //       anchors.rightMargin: 10
      //       anchors.verticalCenter: parent.verticalCenter
      //     }
      //   }
      //
      //   Rectangle {
      //     id: splitter
      //
      //     anchors.horizontalCenter: parent.horizontalCenter
      //     width: parent.width - 20
      //     height: 1
      //
      //     color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1);
      //   }
      //
      //   Item {
      //     width: parent.width
      //     height: details2.height + 20
      //
      //     Column {
      //       id: details2
      //       anchors.left: parent.left
      //       anchors.leftMargin: 10
      //       anchors.verticalCenter: parent.verticalCenter
      //
      //       UI.DefaultText {
      //         text: "Tray-режим"
      //       }
      //
      //       UI.SubheadingText {
      //         text: "Приложение будет отображаться\nкак иконка на панели задач"
      //       }
      //     }
      //
      //     UI.MySwitch {
      //       anchors.right: parent.right
      //       anchors.rightMargin: 10
      //       anchors.verticalCenter: parent.verticalCenter
      //     }
      //   }
      // }
    }
  }
}
