import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

ComboBox {
  id: root
  width: 80
  height: 32

  property color normalColor: themes.GetAccent()
  property color pressedColor: themes.GetAccent2()
  property color textColor: themes.GetControlText()
  property color popupBackground: themes.GetHeaderBackground()

  font.pointSize: 14

  signal indexSelected(int index)

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

  indicator: Item {}

  delegate: ItemDelegate {
    width: root.width
    height: 32
    highlighted: root.highlightedIndex === index

    background: Rectangle {
      anchors.fill: parent
      radius: 8
      color: highlighted ? themes.GetAccent() : "transparent"
    }

    contentItem: Text {
      text: modelData
      color: root.textColor
      verticalAlignment: Text.AlignVCenter
      font.pointSize: 14
    }

    onClicked: {
      root.currentIndex = index
      root.popup.close()
    }
  }

  popup: Popup {
    y: root.height + 4
    width: root.width
    implicitHeight: contentItem.implicitHeight
    padding: 0

    background: Item {
      id: shadowContainer
      width: root.width
      height: listView.implicitHeight

      Rectangle {
        id: popupBackground
        anchors.fill: parent
        radius: 8
        color: root.popupBackground
      }

      DropShadow {
        anchors.fill: popupBackground
        source: popupBackground
        radius: 12
        samples: 16
        horizontalOffset: 0
        verticalOffset: 2
        color: "#66000000"
      }
    }

    contentItem: ListView {
      id: listView
      model: root.delegateModel
      currentIndex: root.highlightedIndex
      clip: true
      interactive: true
      implicitHeight: contentHeight

      highlightRangeMode: ListView.ApplyRange
      highlightMoveDuration: 0

      delegate: Item {
        width: root.width
        height: 32

        Text {
          text: modelData
          anchors.verticalCenter: parent.verticalCenter
          color: themes.GetControlText()
          font.pointSize: 14
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor

          onClicked: {
            root.indexSelected(index)
            root.popup.close()
          }
        }
      }
    }
  }

}
