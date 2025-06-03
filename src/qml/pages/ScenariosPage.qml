import QtQuick
import QtQuick.Layouts
import "../ui/" as UI

Item {
  Layout.fillWidth: true
  Layout.fillHeight: true

  width: parent.width

  Rectangle {
    anchors.fill: parent
    color: "red"  // light gray background, or any visible color
  }

  UI.HeadingText {
    id: heading
    text: "Все сценарии"
  }

  ListView {
    id: scenariouses

    anchors.top: heading.bottom
    anchors.topMargin: 12
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    model: ListModel {
      ListElement { name: "Будильник" }
      ListElement { name: "Сделать все четко" }
    }

    delegate: Item {
      width: parent.width
      height: 48

      anchors.margins: 5

      Rectangle {
        anchors.fill: parent
        color: "#FFF"
        radius: 12
      }

      UI.DefaultText {
        text: name
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 16
      }
    }
  }
}