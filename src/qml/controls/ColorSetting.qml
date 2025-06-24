import QtQuick
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root
  property int currentTab : 0

  width: parent.width
  // height: 1000

  Rectangle {
    anchors.fill: parent
    color: "red"
  }

  // Row {
  //   anchors.left: parent.left
  //   anchors.right: parent.right
  //   height: 36
  //   spacing: 8
  //
  //   Item {
  //     id: colorTab
  //     width: 80; height: parent.height
  //
  //     Rectangle {
  //       anchors.fill: parent
  //       radius: 45
  //       color: root.currentTab === 0 ? themes.GetAccent()
  //         : themes.GetInactive()
  //     }
  //
  //     UI.DefaultText {
  //       anchors.centerIn: parent
  //       text: "Цвета"
  //       font.pointSize: 14
  //       color: themes.GetControlText()
  //     }
  //
  //     MouseArea {
  //       anchors.fill: parent
  //       cursorShape: Qt.PointingHandCursor
  //       onClicked: root.currentTab = 0
  //     }
  //   }
  //
  //   Item {
  //     id: modesTab
  //     width: 80; height: parent.height
  //
  //     Rectangle {
  //       anchors.fill: parent
  //       radius: 45
  //       color: root.currentTab === 1 ? themes.GetAccent()
  //         : themes.GetInactive()
  //     }
  //
  //     UI.DefaultText {
  //       anchors.centerIn: parent
  //       text: "Режимы"
  //       font.pointSize: 14
  //       color: themes.GetControlText()
  //     }
  //
  //     MouseArea {
  //       anchors.fill: parent
  //       cursorShape: Qt.PointingHandCursor
  //       onClicked: root.currentTab = 1
  //     }
  //   }
  // }
  //
  // StackLayout {
  //   anchors.left: parent.left
  //   anchors.right: parent.right
  //   currentIndex: root.currentTab
  //
  //   implicitHeight: currentItem ? currentItem.implicitHeight : 0
  //
  //   Item {
  //     id: colorsPage
  //     implicitHeight: colorsGrid.implicitHeight
  //
  //     Components.AdaptiveGrid {
  //       id: colorsGrid
  //       width: parent.width
  //
  //       count: 20
  //
  //       delegate: Item {
  //         Rectangle {
  //           id: background
  //           anchors.fill: parent
  //           color: themes.GetHeaderBackground()
  //           radius: 16
  //         }
  //
  //         Rectangle {
  //           id: colorDemo
  //           anchors.top: parent.top
  //           anchors.topMargin: 12
  //           anchors.horizontalCenter: parent.horizontalCenter
  //
  //           width: 40
  //           height: 40
  //
  //           radius: 45
  //
  //           color: "blue"
  //         }
  //
  //         UI.DefaultText {
  //           anchors.top: colorDemo.bottom
  //           anchors.topMargin: 4
  //           anchors.horizontalCenter: parent.horizontalCenter
  //
  //           text: "Item " + index
  //           color: themes.GetControlText()
  //         }
  //
  //         MouseArea {
  //           anchors.fill: parent
  //           cursorShape: Qt.PointingHandCursor
  //         }
  //       }
  //     }
  //   }
  //
  //   Item {
  //     implicitHeight: modesGrid.implicitHeight
  //
  //     Components.AdaptiveGrid {
  //       id: modesGrid
  //       width: parent.width
  //
  //       count: 12
  //
  //       delegate: Item {
  //         Rectangle {
  //           id: background
  //           anchors.fill: parent
  //           color: themes.GetHeaderBackground()
  //           radius: 16
  //         }
  //
  //         Rectangle {
  //           id: colorDemo
  //           anchors.top: parent.top
  //           anchors.topMargin: 12
  //           anchors.horizontalCenter: parent.horizontalCenter
  //
  //           width: 40
  //           height: 40
  //
  //           radius: 45
  //
  //           color: "red"
  //         }
  //
  //         UI.DefaultText {
  //           anchors.top: colorDemo.bottom
  //           anchors.topMargin: 4
  //           anchors.horizontalCenter: parent.horizontalCenter
  //
  //           text: "Item " + index
  //           color: themes.GetControlText()
  //         }
  //
  //         MouseArea {
  //           anchors.fill: parent
  //           cursorShape: Qt.PointingHandCursor
  //         }
  //       }
  //     }
  //   }
  // }
}
