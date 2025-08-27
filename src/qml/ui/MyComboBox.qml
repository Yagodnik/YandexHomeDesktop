import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

ComboBox {
  id: root
  width: 80
  height: 32

  property color normalColor: themes.accent
  property color pressedColor: themes.accent2
  property color textColor: themes.controlText
  property color popupBackground: themes.headerBackground

  font.pixelSize: 14
  focusPolicy: Qt.StrongFocus

  signal indexSelected(int index)

  contentItem: Text {
    text: root.displayText
    color: themes.controlText2
    verticalAlignment: Text.AlignVCenter
    leftPadding: 12
    anchors.fill: parent
    wrapMode: Text.WordWrap
    font.pixelSize: 12
  }

  background: Rectangle {
    radius: 8
    color: root.pressed ? root.pressedColor : root.normalColor

    Behavior on color {
      ColorAnimation { duration: 120 }
    }
  }

  indicator: Item {}

  popup: Popup {
    y: root.height + 4
    width: root.width
    padding: 0
    focus: true

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
      model: root.model
      clip: true
      interactive: true
      currentIndex: root.currentIndex
      implicitHeight: Math.min(contentHeight, 200)

      highlightMoveDuration: 0
      highlightRangeMode: ListView.ApplyRange

      delegate: Item {
        width: root.width
        height: 32

        Rectangle {
          anchors.fill: parent
          radius: 8
          color: ListView.isCurrentItem ? themes.accent : "transparent"
        }

        Text {
          text: model.displayText || modelData
          anchors.verticalCenter: parent.verticalCenter
          anchors.left: parent.left
          anchors.leftMargin: 12
          color: ListView.isCurrentItem ? themes.controlText2 : root.textColor
          wrapMode: Text.WordWrap
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor
          hoverEnabled: true

          onClicked: {
            root.currentIndex = index
            root.activated(index)
            root.indexSelected(index)
            root.popup.close()
          }
        }
      }
    }
  }
}
