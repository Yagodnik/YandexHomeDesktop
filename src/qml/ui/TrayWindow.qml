import QtQuick
import QtQuick.Window

Window {
  id: root
  property bool trayModeEnabled: false
  property bool modalDialogOpen: false

  Connections {
    target: root
    function onActiveChanged() {
      // Dialogs can deactivate their parent; clearing item focus alone must not hide it.
      if (!root.active && root.trayModeEnabled && !root.modalDialogOpen)
        root.hide();
    }
  }
}
