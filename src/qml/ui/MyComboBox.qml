import YandexHomeDesktop.Ui as UI
import QtQuick 2.15
import QtQuick.Controls 2.15

ComboBox {
  id: root
  width: 80
  height: 32

  property color normalColor: themes.accent
  property color pressedColor: themes.accent2
  property color textColor: themes.controlText
  property color popupBackground: themes.headerBackground
  property real popupWidth: width

  font.pixelSize: 12
  padding: 0
  leftPadding: 12
  rightPadding: 12
  focusPolicy: Qt.StrongFocus
  hoverEnabled: true

  HoverHandler { cursorShape: Qt.PointingHandCursor }

  signal indexSelected(int index)

  contentItem: Text {
    objectName: "comboBoxLabel"
    text: root.displayText
    color: themes.controlText2
    verticalAlignment: Text.AlignVCenter
    wrapMode: Text.NoWrap
    elide: Text.ElideRight
    font: root.font
  }

  background: Rectangle {
    radius: 8
    color: root.pressed ? root.pressedColor : root.normalColor

    Behavior on color {
      ColorAnimation { duration: 120 }
    }
  }

  indicator: Item {}

  delegate: Item {
    id: option
    objectName: "comboBoxOption-" + index
    readonly property bool isSelected: root.currentIndex === index
    width: root.popup.width
    height: 32

    Rectangle {
      anchors.fill: parent
      radius: 8
      color: option.isSelected ? themes.accent : "transparent"
    }

    Text {
      objectName: "comboBoxOptionLabel"
      text: root.textAt(index)
      anchors.fill: parent
      anchors.leftMargin: root.leftPadding
      anchors.rightMargin: root.rightPadding
      color: option.isSelected ? themes.controlText2 : root.textColor
      verticalAlignment: Text.AlignVCenter
      wrapMode: Text.NoWrap
      elide: Text.ElideRight
      font: root.font
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

  popup: Popup {
    y: root.height + 4
    x: root.width - width
    width: root.popupWidth
    padding: 0
    focus: true

    background: Item {
      id: shadowContainer
      width: root.popup.width
      height: listView.implicitHeight

      Rectangle {
        id: popupBackground
        anchors.fill: parent
        radius: 8
        color: root.popupBackground
      }

      UI.Shadow {
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
      clip: true
      interactive: true
      currentIndex: root.currentIndex
      implicitHeight: Math.min(contentHeight, 200)

      highlightMoveDuration: 0
      highlightRangeMode: ListView.ApplyRange

    }
  }
}
