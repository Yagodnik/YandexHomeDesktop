import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI

UI.AnimatedDialog {
  id: root
  parent: Overlay.overlay
  modal: true
  focus: true
  width: Math.min(336, parent.width - 24)
  x: (parent.width - width) / 2
  y: (parent.height - height) / 2
  padding: 20
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

  property string capabilityTitle
  property int currentCombination: 0
  property int draftCombination: 0
  property var formatCombination: function(combination) { return ""; }
  readonly property string draftSequence: formatCombination(draftCombination)
  property string errorMessage: ""
  signal keyRecorded(int key, int modifiers)
  signal combinationAccepted(int combination)

  onAboutToShow: draftCombination = currentCombination
  onOpened: recorder.forceActiveFocus()

  function record(event) {
    event.accepted = true;
    if (event.isAutoRepeat) return;
    if (event.key === Qt.Key_Escape) { reject(); return; }
    if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
      draftCombination = 0;
      return;
    }
    if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift
        || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta || event.key === Qt.Key_AltGr) return;
    // Keep ordinary Tab navigation available to reach the dialog actions.
    if (event.key === Qt.Key_Backtab
        || event.key === Qt.Key_Tab && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier))) {
      event.accepted = false;
      return;
    }
    root.keyRecorded(event.key, event.modifiers);
  }

  background: Rectangle {
    color: themes.headerBackground
    radius: 16
    border.width: 1
    border.color: themes.trackColor
  }

  contentItem: ColumnLayout {
    spacing: 12

    UI.HeadingText {
      Layout.fillWidth: true
      font.pixelSize: 20
      text: qsTr("Сочетание клавиш")
    }

    UI.DefaultText {
      Layout.fillWidth: true
      text: root.capabilityTitle
      elide: Text.ElideRight
      font.pixelSize: 13
      color: themes.inactive
    }

    FocusScope {
      id: recorder
      objectName: "keybindRecorder"
      Layout.fillWidth: true
      implicitHeight: 70
      activeFocusOnTab: true
      focus: true
      Accessible.role: Accessible.EditableText
      Accessible.name: qsTr("Нажмите клавишу или сочетание")
      Accessible.description: root.draftSequence
      Keys.onShortcutOverride: function(event) { event.accepted = true; }
      Keys.onPressed: function(event) { root.record(event); }

      Rectangle {
        anchors.fill: parent
        radius: 12
        property color accent: themes.accent
        color: Qt.rgba(accent.r, accent.g, accent.b, 0.06)
        border.width: recorder.activeFocus ? 2 : 1
        border.color: recorder.activeFocus ? themes.accent : themes.inactive
      }

      UI.DefaultText {
        anchors.fill: parent
        anchors.margins: 12
        text: root.draftSequence.length > 0 ? root.draftSequence : qsTr("Нажмите клавиши…")
        font.pixelSize: root.draftSequence.length > 0 ? 22 : 15
        font.bold: root.draftSequence.length > 0
        color: root.draftSequence.length > 0 ? themes.mainText : themes.inactive
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
      }

      TapHandler { onTapped: recorder.forceActiveFocus() }
    }

    UI.DefaultText {
      Layout.fillWidth: true
      text: qsTr("Esc — отмена · Backspace — очистить")
      font.pixelSize: 11
      color: themes.inactive
      wrapMode: Text.WordWrap
    }

    UI.DefaultText {
      Layout.fillWidth: true
      text: qsTr("Сочетание сохранится в настройках. Управление устройствами с клавиатуры пока недоступно.")
      font.pixelSize: 12
      color: themes.mainText
      opacity: 0.65
      wrapMode: Text.WordWrap
    }

    UI.DefaultText {
      objectName: "keybindConflictMessage"
      Layout.fillWidth: true
      visible: root.errorMessage.length > 0
      text: root.errorMessage
      color: themes.mainText
      font.pixelSize: 12
      wrapMode: Text.WordWrap
      Accessible.role: Accessible.AlertMessage
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: 8

      EditorButton {
        objectName: "keybindClear"
        text: qsTr("Очистить")
        enabled: root.draftSequence.length > 0
        onClicked: {
          root.draftCombination = 0;
          recorder.forceActiveFocus();
        }
      }

      Item { Layout.fillWidth: true }

      EditorButton {
        objectName: "keybindCancel"
        text: qsTr("Отмена")
        onClicked: root.reject()
      }

      EditorButton {
        objectName: "keybindSave"
        text: qsTr("Готово")
        primary: true
        enabled: root.errorMessage.length === 0
        onClicked: {
          root.combinationAccepted(root.draftCombination);
        }
      }
    }
  }

  component EditorButton: Button {
    id: button
    property bool primary: false
    implicitWidth: buttonLabel.implicitWidth + 20
    implicitHeight: 36
    hoverEnabled: true
    opacity: enabled ? 1 : 0.35
    HoverHandler { cursorShape: Qt.PointingHandCursor }
    background: Rectangle {
      radius: 8
      color: button.primary ? (button.down ? themes.accent2 : themes.accent)
        : button.hovered ? themes.background : "transparent"
      border.width: button.visualFocus ? 2 : 0
      border.color: themes.accent
    }
    contentItem: UI.DefaultText {
      id: buttonLabel
      text: button.text
      font.pixelSize: 13
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
      color: button.primary ? themes.controlText2 : themes.mainText
    }
  }
}
