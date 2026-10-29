import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
  id: root
  property bool trayModeEnabled: false
  property bool notchModeEnabled: false
  property bool popupMenuInHeader: false
  property bool popupQuitVisible: true
  readonly property bool popupModeEnabled: trayModeEnabled || notchModeEnabled
  signal quitRequested()

  ToolButton {
    id: quitButton
    objectName: "popupQuitButton"
    property color foregroundColor: themes.inactive
    anchors.top: parent.top
    anchors.topMargin: root.popupMenuInHeader ? 16 : 0
    anchors.right: parent.right
    anchors.rightMargin: root.popupMenuInHeader ? 12 : 0
    width: 32
    height: 28
    visible: root.popupModeEnabled && root.popupQuitVisible
    z: 1000
    text: qsTr("Выйти из приложения")
    Accessible.name: text
    onClicked: root.quitRequested()
    HoverHandler { cursorShape: Qt.PointingHandCursor }
    ToolTip.visible: hovered
    ToolTip.delay: 500
    ToolTip.text: text
    contentItem: Text {
      text: "×"
      color: quitButton.foregroundColor
      font.pixelSize: 24
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
      radius: 6
      color: Qt.rgba(quitButton.foregroundColor.r, quitButton.foregroundColor.g,
                     quitButton.foregroundColor.b, quitButton.down ? 0.22 : quitButton.hovered ? 0.12 : 0)
      border.width: quitButton.visualFocus ? 2 : 0
      border.color: quitButton.foregroundColor
    }
  }
  property bool modalDialogOpen: false

  Connections {
    target: root
    function onActiveChanged() {
      // Dialogs can deactivate their parent; clearing item focus alone must not hide it.
      if (!root.active && root.popupModeEnabled && !root.modalDialogOpen)
        root.hide();
    }
  }
}
