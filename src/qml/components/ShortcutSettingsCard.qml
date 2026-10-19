import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  property bool shortcutsEnabled: true
  property bool backgroundEnabled: false
  property bool busy: false
  property string statusText
  property string errorText
  signal enabledToggled(bool enabled)
  signal backgroundToggled(bool enabled)
  implicitHeight: rows.implicitHeight
  UI.CardSurface { anchors.fill: parent }
  Column {
    id: rows
    width: root.width
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
      x: 10
      width: parent.width - 20
      text: root.statusText
      visible: text.length > 0
      wrapMode: Text.Wrap
    }
    UI.SubheadingText {
      x: 10
      width: parent.width - 20
      text: root.errorText
      visible: text.length > 0
      wrapMode: Text.Wrap
    }
    Item { width: 1; height: 10; visible: root.statusText.length > 0 || root.errorText.length > 0 }
  }
}
