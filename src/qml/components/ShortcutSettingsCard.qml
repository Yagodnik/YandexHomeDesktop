import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  property bool shortcutsEnabled: false
  property bool backgroundEnabled: false
  property bool busy: false
  property string statusText
  property string errorText
  signal enabledToggled(bool enabled)
  signal backgroundToggled(bool enabled)
  implicitHeight: rows.implicitHeight + 20
  UI.CardSurface { anchors.fill: parent }
  Column {
    id: rows
    width: root.width - 20
    x: 10
    y: 10
    spacing: 8
    UI.SettingSwitchRow {
      objectName: "shortcutsEnabledRow"
      width: parent.width
      title: qsTr("Глобальные сочетания")
      description: qsTr("Управлять устройствами и запускать сценарии,\nдаже когда другое приложение в фокусе")
      checked: root.shortcutsEnabled
      enabled: !root.busy || root.shortcutsEnabled
      onToggled: function(checked) { root.enabledToggled(checked); }
    }
    UI.SettingSwitchRow {
      objectName: "shortcutsBackgroundRow"
      width: parent.width
      title: qsTr("После закрытия приложения")
      description: qsTr("Оставлять службу сочетаний работающей\nбез окна и иконки в трее")
      checked: root.backgroundEnabled
      enabled: root.shortcutsEnabled
      onToggled: function(checked) { root.backgroundToggled(checked); }
    }
    UI.SubheadingText {
      width: parent.width
      text: root.statusText
      wrapMode: Text.Wrap
    }
    UI.SubheadingText {
      width: parent.width
      text: root.errorText
      visible: text.length > 0
      wrapMode: Text.Wrap
    }
  }
}
