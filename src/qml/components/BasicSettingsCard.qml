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
  property string projectLinkText
  signal trayModeToggled(bool enabled)
  signal themeSelected(int index)
  signal projectLinkClicked()

  UI.CardSurface { anchors.fill: parent }

  Column {
    id: rows
    width: root.width

    UI.SettingSwitchRow {
      width: parent.width
      title: root.trayTitle
      description: root.trayDescription
      checked: root.trayModeEnabled
      onToggled: function(checked) { root.trayModeToggled(checked); }
    }

    UI.SettingChoiceRow {
      width: parent.width
      title: root.themeTitle
      choices: root.themeNames
      currentIndex: root.currentTheme
      onActivated: function(index) { root.themeSelected(index); }
    }

    UI.SettingLinkRow {
      width: parent.width
      text: root.projectLinkText
      onClicked: root.projectLinkClicked()
    }
  }
}
