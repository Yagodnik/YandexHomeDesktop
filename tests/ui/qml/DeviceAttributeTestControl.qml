import QtQuick

Rectangle {
  objectName: model.controlId
  readonly property string keybindTitle: model.controlId
  height: model.controlHeight
  color: "#cccccc"
}
