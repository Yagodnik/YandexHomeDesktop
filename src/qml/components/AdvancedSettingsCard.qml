import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: rows.implicitHeight + 20
  property string title
  property string description
  property string statusText
  property string errorText
  property bool restEnabled: false
  property bool busy: false
  signal restToggled(bool enabled)

  UI.CardSurface { anchors.fill: parent }

  Column {
    id: rows
    width: root.width - 20
    x: 10
    y: 10
    spacing: 8

    UI.SettingSwitchRow {
      objectName: "restSettingRow"
      width: parent.width
      title: root.title
      description: root.description
      checked: root.restEnabled
      enabled: !root.busy
      onToggled: function(checked) { root.restToggled(checked); }
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
