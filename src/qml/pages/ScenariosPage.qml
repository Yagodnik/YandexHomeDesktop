import QtQuick 2.15
import QtQuick.Layouts 2.15
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Item {
  Flickable {
    id: flickable
    anchors.fill: parent
    contentWidth: parent.width
    contentHeight: contentItem.implicitHeight

    interactive: true
    flickableDirection: Flickable.VerticalFlick

    Column {
      id: contentItem
      width: flickable.width

      spacing: 12

      UI.HeadingText {
        id: heading
        text: "Все сценарии"
        width: parent.width
      }

      ListView {
        id: scenariouses
        width: parent.width
        // height: 48 * model.count + (scenariouses.spacing * (model.count - 1))
        implicitHeight: contentHeight

        // model: ListModel {
        //   ListElement { name: "Будильник"; acting: false }
        //   ListElement { name: "Сделать все четко"; acting: false }
        //   ListElement { name: "Будильник"; acting: true }
        //   ListElement { name: "Сделать все четко"; acting: true }
        //   ListElement { name: "Будильник"; acting: false }
        //   ListElement { name: "Сделать все четко"; acting: false }
        //   ListElement { name: "Будильник"; acting: false }
        //   ListElement { name: "Сделать все четко"; acting: false }
        // }
        model: scenariosModel

        Component.onCompleted: {
          scenariosModel.RequestData();
        }

        spacing: 5
        interactive: false
        clip: true

        delegate: Item {
          id: delegateItem
          width: scenariouses.width
          height: 48

          DropShadow {
            anchors.fill: background
            source: background
            horizontalOffset: 0
            verticalOffset: 2
            radius: 6
            samples: 16
            color: Qt.rgba(0, 32 / 255, 128 / 255, 0.04)
          }

          Rectangle {
            id: background
            anchors.fill: parent
            color: "#FFF"
            radius: 16
          }

          UI.DefaultText {
            text: name
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 16
          }

          property var show: false

          UI.MyProgressIndicator {
            width: 30
            height: 30
            strokeWidth: 2
            anchors.right: parent.right
            anchors.rightMargin: 9
            anchors.verticalCenter: parent.verticalCenter
            visible: acting
          }

          Image {
            id: startButton
            source: "qrc:/images/play.svg"

            antialiasing: true
            layer.enabled: true
            layer.smooth: true
            layer.samples: 8

            fillMode: Image.PreserveAspectFit
            scale: 0.5

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 8

            MouseArea {
              anchors.fill: parent
              cursorShape: Qt.PointingHandCursor

              onClicked: {
                console.log("Executing " + scenario_id);
                yandex_api.ExecuteScenario(scenario_id);
              }
            }
          }
        }
      }
    }
  }
}
