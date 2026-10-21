import QtQuick
import YandexHomeDesktop.Ui as UI

Item {
  id: root
  implicitHeight: rows.implicitHeight
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
    width: root.width

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
