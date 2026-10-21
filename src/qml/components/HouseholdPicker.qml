import QtQuick
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.Components as Components

Item {
  id: root
  z: 500
  property string title
  property var sourceModel: null
  property string currentHousehold
  property alias loading: dialog.loading
  readonly property bool opened: dialog.myVisible
  signal householdSelected(string householdId)

  function open() { dialog.open(); }
  function close() { dialog.close(); }

  Rectangle {
    objectName: "householdBackdrop"
    anchors.fill: parent
    color: "#80000000"
    visible: root.opened

    MouseArea {
      anchors.fill: parent
      onClicked: root.close()
    }
  }

  UI.SelectDialog {
    id: dialog
    title: root.title
    model: root.sourceModel
    loading: true

    loadingDelegate: UI.LoadingPane {
      active: root.loading
    }

    delegate: Components.HouseholdDelegate {
      objectName: "household-" + model.householdId
      width: parent.width
      title: model.name
      selected: model.householdId === root.currentHousehold
      showSeparator: model.index < root.sourceModel.count - 1
      onClicked: root.householdSelected(model.householdId)
    }
  }
}
