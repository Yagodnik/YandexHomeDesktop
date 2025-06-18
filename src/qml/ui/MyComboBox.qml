import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ComboBox {
  id: root
  width: 80
  height: 32

  property color normalColor: Qt.rgba(104 / 255, 57 / 255, 207 / 255, 1)
  property color pressedColor: Qt.rgba(84 / 255, 45 / 255, 168 / 255, 1)
  property color textColor: "white"
  property color popupBackground: "#f8f8f8"
  property color popupHighlight: Qt.rgba(220 / 255, 220 / 255, 255 / 255, 1)

  font.pointSize: 14

  contentItem: Text {
    text: root.displayText
    color: root.textColor
    verticalAlignment: Text.AlignVCenter
    anchors.centerIn: root
    leftPadding: 12
  }

  background: Rectangle {
    radius: 8
    color: root.pressed ? root.pressedColor : root.normalColor

    Behavior on color {
      ColorAnimation { duration: 120 }
    }
  }

  indicator: Item {
    width: 0
    height: 0
  }

  popup: Popup {
    y: root.height + 4
    width: root.width
    implicitHeight: contentItem.implicitHeight
    padding: 0

    background: Rectangle {
      radius: 8
      color: root.popupBackground
    }

    contentItem: ListView {
      model: root.delegateModel
      currentIndex: root.highlightedIndex
      clip: true
      interactive: true
      implicitHeight: contentHeight

      delegate: Item {
        id: delegateRect
        width: root.width
        height: 32

        Rectangle {
          anchors.fill: parent
          color: ListView.isCurrentItem ? root.popupHighlight : "transparent"
          radius: 16
        }

        Text {
          text: modelData
          anchors.verticalCenter: parent.verticalCenter

          color: "#000"
          font.pointSize: 14
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor

          onClicked: {
            root.currentIndex = index
            root.popup.close()
          }
        }
      }
    }
  }
}
