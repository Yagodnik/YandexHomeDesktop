import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: rows.implicitHeight
  property string trayTitle
  property string trayDescription
  property bool trayModeEnabled: false
  property string themeTitle
  property var themeNames
  property int currentTheme: 0
  property string keybindsTitle
  property string keybindsDescription
  property bool showKeybinds: true
  property bool notchAvailable: false
  property bool notchEnabled: false
  property string notchTitle
  property string notchDescription
  signal notchToggled(bool enabled)
  property string projectLinkText
  signal trayModeToggled(bool enabled)
  signal themeSelected(int index)
  signal keybindsToggled(bool visible)
  signal projectLinkClicked()

  UI.CardSurface { anchors.fill: parent }

  Column {
    id: rows
    width: root.width

    UI.SettingSwitchRow {
      objectName: "traySettingRow"
      width: parent.width
      title: root.trayTitle
      description: root.trayDescription
      checked: root.trayModeEnabled
      onToggled: function(checked) { root.trayModeToggled(checked); }
    }

    UI.SettingSwitchRow {
      objectName: "notchSettingRow"
      visible: root.notchAvailable
      width: parent.width
      title: root.notchTitle
      description: root.notchDescription
      checked: root.notchEnabled
      onToggled: function(checked) { root.notchToggled(checked); }
    }

    UI.SettingChoiceRow {
      width: parent.width
      title: root.themeTitle
      choices: root.themeNames
      currentIndex: root.currentTheme
      onActivated: function(index) { root.themeSelected(index); }
    }

    UI.SettingSwitchRow {
      objectName: "keybindsSettingRow"
      width: parent.width
      title: root.keybindsTitle
      description: root.keybindsDescription
      checked: root.showKeybinds
      onToggled: function(checked) { root.keybindsToggled(checked); }
    }

    UI.SettingLinkRow {
      width: parent.width
      text: root.projectLinkText
      onClicked: root.projectLinkClicked()
    }
  }
}
