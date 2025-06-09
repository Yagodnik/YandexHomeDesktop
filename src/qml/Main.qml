import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.platform
import "./ui/" as UI
import "./components" as Components
import "./pages/" as Pages

Window {
  width: 350
  height: 460
  visible: true
  title: qsTr("Yandex Home Desktop")
  // flags: Qt.FramelessWindowHint

  Rectangle {
    anchors.fill: parent
    color: Qt.rgba(242 / 255, 243 / 255, 245 / 255, 1)
  }

  Column {
    id: col
    anchors.centerIn: parent
    spacing: 10

    UI.AnimatedText {
      text: "Yandex Home Desktop"
      color: "#6839CF"
      pointSize: 24
      anchors.horizontalCenter: col.horizontalCenter
    }

    UI.AnimatedText {
      text: qsTr("Необходимо войти в аккаунт, чтобы\nприложение могло получить доступ\nк вашим устройствам")

      color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1);
      pointSize: 16
      anchors.horizontalCenter: col.horizontalCenter
    }

    UI.MyButton {
      text: "Авторизоваться"
      anchors.horizontalCenter: col.horizontalCenter
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

        // width: 20
        // height: 20
        fillMode: Image.PreserveAspectFit
        scale: 0.9

        // anchors.verticalCenter: githubFooter.verticalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: githubLink.left
        anchors.rightMargin: 8
      }

      UI.DefaultText {
        id: githubLink

        color: Qt.rgba(145 / 255, 156 / 255, 181 / 255, 1);
        // anchors.horizontalCenter: githubFooter.horizontalCenter
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
