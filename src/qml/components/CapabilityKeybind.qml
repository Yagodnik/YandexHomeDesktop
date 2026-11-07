import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import YandexHomeDesktop.Ui as UI
import YandexHomeDesktop.ViewModels

AbstractButton {
  id: root

  property string capabilityTitle
  property string bindingDescription: capabilityTitle
  property KeybindSettingsModel keybindSettings: null
  property var bindingTarget: ({})
  property string deviceName
  readonly property int combination: {
    if (!keybindSettings) return 0;
    keybindSettings.revision; // Re-evaluate after settings or account changes.
    return keybindSettings.GetCombination(bindingTarget);
  }
  readonly property string sequence: keybindSettings ? keybindSettings.DisplayCombination(combination) : ""
  property string saveError: ""
  property bool recording: false
  Component.onDestruction: { if (recording && keybindSettings) keybindSettings.EndRecording(); }
  property string editingAccount: ""
  property string actionLabel: ""
  property bool compact: false
  readonly property alias editor: editor

  implicitWidth: label.visible ? Math.min(220, label.implicitWidth + 42) : 35
  implicitHeight: 26
  hoverEnabled: true
  opacity: hovered || visualFocus || editor.visible ? 1 : 0.55
  Accessible.name: sequence.length > 0
    ? qsTr("Изменить сочетание %1 для «%2»").arg(sequence).arg(capabilityTitle)
    : qsTr("Назначить сочетание для «%1»").arg(capabilityTitle)

  HoverHandler { cursorShape: Qt.PointingHandCursor }
  Behavior on opacity { NumberAnimation { duration: 120 } }

  background: Rectangle {
    radius: 8
    color: themes.headerBackground
    border.width: 1
    property color outline: root.hovered || root.visualFocus ? themes.accent : themes.inactive
    border.color: Qt.rgba(outline.r, outline.g, outline.b, root.visualFocus ? 1 : 0.4)
  }

  contentItem: RowLayout {
    spacing: 6

    // A small keycap keeps the attachment recognizable without a new asset.
    Rectangle {
      Layout.leftMargin: 9
      Layout.rightMargin: label.visible ? 0 : 9
      implicitWidth: 17
      implicitHeight: 13
      radius: 3
      color: "transparent"
      border.color: themes.inactive

      Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 3
        width: 9
        height: 1
        color: themes.inactive
      }
    }

    UI.DefaultText {
      id: label
      Layout.fillWidth: true
      Layout.rightMargin: 9
      font.pixelSize: 11
      visible: !root.compact || root.sequence.length > 0
      color: root.hovered || root.visualFocus ? themes.accent : themes.mainText
      text: (root.actionLabel.length > 0 ? root.actionLabel + "  " : "")
        + (root.sequence.length > 0 ? root.sequence : qsTr("Назначить"))
      elide: Text.ElideRight
    }
  }

  ToolTip.visible: hovered
  ToolTip.delay: 600
  ToolTip.text: Accessible.name
  onClicked: {
    saveError = "";
    editingAccount = keybindSettings ? keybindSettings.accountId : "";
    editor.open();
  }
  onVisibleChanged: { if (!visible) editor.close(); }

  Connections {
    target: root.keybindSettings
    function onChanged() {
      if (editor.visible && root.editingAccount !== root.keybindSettings.accountId) editor.reject();
    }
  }

  UI.KeybindEditor {
    id: editor
    objectName: "keybindEditor"
    capabilityTitle: root.capabilityTitle
    currentCombination: root.combination
    recordingReady: !root.keybindSettings || root.keybindSettings.recordingReady
    onAboutToShow: { root.recording = true; if (root.keybindSettings) root.keybindSettings.BeginRecording(); }
    onClosed: { if (root.recording && root.keybindSettings) root.keybindSettings.EndRecording(); root.recording = false; }
    formatCombination: function(combination) { return root.keybindSettings ? root.keybindSettings.DisplayCombination(combination) : ""; }
    errorMessage: {
      if (!root.keybindSettings) return qsTr("Настройки сочетаний недоступны.");
      root.keybindSettings.revision;
      return root.keybindSettings.recordingError || root.saveError || root.keybindSettings.ConflictMessage(root.bindingTarget, draftCombination);
    }
    onDraftCombinationChanged: root.saveError = ""
    onKeyRecorded: function(key, modifiers) {
      if (root.keybindSettings) draftCombination = root.keybindSettings.MakeCombination(key, modifiers);
    }
    onCombinationAccepted: function(combination) {
      if (!root.keybindSettings || root.editingAccount !== root.keybindSettings.accountId) return;
      if (root.keybindSettings.Save(root.bindingTarget, combination, root.deviceName, root.bindingDescription)) accept();
      else root.saveError = root.keybindSettings.lastError;
    }
  }
}
