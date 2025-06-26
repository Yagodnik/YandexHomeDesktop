import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import YandexHomeDesktop.Ui as UI

Dialog {
  id: errorDialog
  modal: true
  focus: true
  x: (parent.width - width) / 2
  y: (parent.height - height) / 2
  padding: 20
  width: 320
  visible: false
  opacity: 0
  scale: 0.9
  property var dialogTitle
  property var dialogMessage
  property bool closing: false

  function openDialog() {
    visible = true
    closing = false
    opacity = 0
    scale = 0.9
    openAnim.start()
  }

  function closeAnimated() {
    if (closing) return
    closing = true
    closeAnim.start()
  }

  background: Item {
    Rectangle {
      id: backgroundRect
      color: themes.headerBackground
      anchors.fill: parent
      radius: 16
    }

    DropShadow {
      anchors.fill: parent
      horizontalOffset: 0
      verticalOffset: 4
      radius: 12
      samples: 16
      color: "#33000000"
      source: backgroundRect
    }
  }

  contentItem: ColumnLayout {
    spacing: 12

    UI.DefaultText {
      text: errorDialog.dialogTitle
      font.pixelSize: 18
      font.bold: true
      color: themes.accent
    }

    UI.DefaultText {
      text: errorDialog.dialogMessage
      wrapMode: Text.WordWrap
      color: themes.controlText
      Layout.fillWidth: true
    }

    UI.MyButton {
      text: "Ок"
      Layout.alignment: Qt.AlignRight
      onClicked: errorDialog.closeAnimated()
    }
  }

  ParallelAnimation {
    id: openAnim
    PropertyAnimation { target: errorDialog; property: "opacity"; from: 0; to: 1; duration: 200 }
    PropertyAnimation { target: errorDialog; property: "scale"; from: 0.9; to: 1; duration: 200; easing.type: Easing.OutBack }
  }

  SequentialAnimation {
    id: closeAnim
    ParallelAnimation {
      PropertyAnimation {
        target: errorDialog; property: "opacity";
        from: 1;
        to: 0; duration: 200
      }
      PropertyAnimation {
        target: errorDialog; property: "scale";
        from: 1;
        to: 0.9; duration: 200; easing.type: Easing.InBack
      }
    }
    ScriptAction {
      script: {
        errorDialog.visible = false
        errorDialog.closing = false
        errorDialog.opacity = 1
        errorDialog.scale = 1
      }
    }
  }
}
