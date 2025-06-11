import QtQuick
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  Rectangle {
    anchors.fill: parent
    color: Qt.rgba(242 / 255, 243 / 255, 245 / 255, 1)
  }

  Connections {
    target: authorizationService

    function onAuthorized() {
      router.navigateTo("main");
    }
  }

  Item {
    id: content
    anchors.centerIn: parent

    Rectangle {
      id: background
      color: Qt.rgba(255 / 255, 255 / 255, 255 / 255, 1)
      radius: 16
      anchors.centerIn: parent

      Column {
        id: col
        spacing: 10
        anchors.centerIn: parent

        UI.AnimatedText {
          text: "Yandex Home Desktop"
          color: "#6839CF"
          pointSize: 24

          anchors.horizontalCenter: col.horizontalCenter
        }

        UI.AnimatedText {
          text: qsTr("Необходимо войти в аккаунт, чтобы\nприложение могло получить доступ\nк вашим устройствам")
          color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1)
          pointSize: 16

          anchors.horizontalCenter: col.horizontalCenter
        }

        UI.MyButton {
          text: "Авторизоваться"

          anchors.horizontalCenter: col.horizontalCenter

          onClicked: authorizationService.AttemptAuthorization();
        }
      }

      width: col.implicitWidth + 40
      height: col.implicitHeight + 40
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
  }

  Item {
    id: githubFooter

    height: 48
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    Item {
      anchors.horizontalCenter: githubFooter.horizontalCenter
      anchors.verticalCenter: githubFooter.verticalCenter

      Image {
        id: githubLogo
        source: "qrc:/images/login_github.svg"

        antialiasing: true
        layer.enabled: true
        layer.smooth: true
        layer.samples: 8

        fillMode: Image.PreserveAspectFit
        scale: 0.9

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: githubLink.left
        anchors.rightMargin: 8
      }

      UI.DefaultText {
        id: githubLink

        color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1);

        anchors.verticalCenter: parent.verticalCenter

        text: "GitHub"

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor

          onClicked: {
            Qt.openUrlExternally("https://github.com/Yagodnik/YandexHomeDesktop")
          }
        }
      }
    }
  }
}