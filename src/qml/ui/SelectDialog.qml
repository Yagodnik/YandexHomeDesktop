import QtQuick
import QtQuick.Shapes
import YandexHomeDesktop.Ui as UI

Item {
  id: root

  property var title
  property var model
  property var loading
  property Component delegate
  property Component loadingDelegate
  property var selectedIndex: -1

  property var myVisible: false

  state: "closed"
  function open() {
    myVisible = true
    state = "opened";
  }
  function close() {
    myVisible = false
    state = "closed";
  }

  visible: true
  anchors.left: parent.left
  anchors.right: parent.right
  z: 600
  height: loading ? 120 : contentColumn.implicitHeight + 70

  y: parent.height

  states: [
    State {
      name: "opened"

      PropertyChanges {
        target: root
        visible: true
        y: parent.height - root.height
      }
    },

    State {
      name: "closed"

      PropertyChanges {
        target: root
        visible: false
        y: parent.height
      }
    }
  ]

  transitions: [
    Transition {
      from: "opened"
      to: "closed"

      SequentialAnimation {
        NumberAnimation {
          target: root
          properties: "y"
          duration: 300
          easing.type: Easing.InOutQuad
        }

        NumberAnimation {
          target: root
          properties: "visible"
          duration: 0
        }
      }
    },

    Transition {
      from: "closed"
      to: "opened"

      SequentialAnimation {
        NumberAnimation {
          target: root
          properties: "visible"
          duration: 0
        }

        NumberAnimation {
          target: root
          properties: "y"
          duration: 300
          easing.type: Easing.InOutQuad
        }
      }
    }
  ]

  Shape {
    id: background
    anchors.fill: parent

    ShapePath {
      id: shape
      fillColor: themes.headerBackground
      strokeWidth: 0

      property real w: background.width
      property real h: background.height
      property real r: 16

      startX: r
      startY: 0

      PathLine { x: shape.w - shape.r; y: 0 }
      PathQuad {
        x: shape.w
        y: shape.r
        controlX: shape.w
        controlY: 0
      }

      PathLine { x: shape.w; y: shape.h }
      PathLine { x: 0; y: shape.h }
      PathLine { x: 0; y: shape.r }
      PathQuad {
        x: shape.r
        y: 0
        controlX: 0
        controlY: 0
      }
    }
  }

  UI.HeadingText {
    id: selectTitle
    text: title

    font.pointSize: 24
    font.bold: true

    anchors.left: parent.left
    anchors.leftMargin: 16

    anchors.top: parent.top
    anchors.topMargin: 12
  }

  Column {
    visible: !root.loading

    id: contentColumn
    anchors.top: selectTitle.bottom
    anchors.topMargin: 12
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16
    spacing: 8

    Repeater {
      model: root.model
      delegate: root.delegate
    }
  }

  Item {
    visible: root.loading

    anchors.top: selectTitle.bottom
    anchors.topMargin: 12
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16

    Loader {
      anchors.topMargin: 40

      anchors.fill: parent
      sourceComponent: root.loadingDelegate
    }
  }
}